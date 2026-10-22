/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */
#include "LuaStaticCheck.h"

#include <QCoreApplication>

#include "lua.hpp"
#include "lstate.h"     // gco2ts / gco2lcl 等 GCObject 转换宏（lobject.h 的 tsvalue 等依赖此文件）
#include "lobject.h"    // Proto / LClosure / TString / TValue / getstr / tsvalue / ttisstring
#include "lopcodes.h"   // OpCode / Instruction / GET_OPCODE / GETARG_A,B,C / OP_GETTABUP / OP_SETTABUP

#include <QStringList>
#include <QByteArray>
#include <cstring>

static_assert(LUA_VERSION_NUM == 504,
              "LuaStaticCheck 依赖 Lua 5.4 字节码/Proto 布局,升级 Lua 时需复核本文件");

namespace {

// 上值 idx 是否名为 "_ENV"(全局访问的标志)
bool isEnvUpval(const Proto* p, int idx)
{
    if (idx < 0 || idx >= p->sizeupvalues) return false;
    const TString* n = p->upvalues[idx].name;
    return n && std::strcmp(getstr(n), "_ENV") == 0;
}

// 常量 idx 是否字符串
bool constIsString(const Proto* p, int idx)
{
    return idx >= 0 && idx < p->sizek && ttisstring(&p->k[idx]);
}

// 取字符串常量。
// 注：必须先把 tsvalue 结果存入局部，并用显式长度构造 QString。
// 否则内联 getstr(tsvalue(...)) 的静态类型是 char[1](TString::contents 柔性数组),
// 会绑定 Qt6 的 QByteArrayView/数组重载 fromUtf8,按数组声明长度(=1)截断成首字符。
QString constStr(const Proto* p, int idx)
{
    const TString* ts = tsvalue(&p->k[idx]);
    return QString::fromUtf8(getstr(ts), static_cast<qsizetype>(tsslen(ts)));
}

// 递归扫描:GETTABUP _ENV K[name] → referenced;SETTABUP _ENV K[name] → defined。
// 递归 p->p[] 嵌套函数 → 路径无关(未调用/不执行分支也已编入)。
// 注：GETARG_B/GETARG_C 内含 check_exp(checkopm(i,iABC),...)；当 LUAI_ASSERT
// 未定义时 check_exp 退化为 (e)，无断言开销。OP_GETTABUP/OP_SETTABUP 均为
// iABC 格式，即使断言生效也会通过。
void scanProto(const Proto* p, QSet<QString>& defined, QSet<QString>& referenced)
{
    for (int pc = 0; pc < p->sizecode; ++pc) {
        const Instruction instr = p->code[pc];
        switch (GET_OPCODE(instr)) {
        case OP_GETTABUP: {              // R[A] := UpVal[B][K[C]:shortstring]
            const int b = GETARG_B(instr);   // 上值索引
            const int c = GETARG_C(instr);   // 键常量索引
            if (isEnvUpval(p, b) && constIsString(p, c))
                referenced.insert(constStr(p, c));
            break;
        }
        case OP_SETTABUP: {              // UpVal[A][K[B]:shortstring] := RK(C)
            const int a  = GETARG_A(instr);  // 上值索引
            const int bk = GETARG_B(instr);  // 键常量索引
            if (isEnvUpval(p, a) && constIsString(p, bk))
                defined.insert(constStr(p, bk));
            break;
        }
        default: break;
        }
    }
    for (int i = 0; i < p->sizep; ++i)
        scanProto(p->p[i], defined, referenced);
}

} // namespace

namespace LuaStaticCheck {

bool run(const QString& script, const QSet<QString>& knownFunctions, QString& errorMsg)
{
    lua_State* L = luaL_newstate();
    if (!L) { errorMsg = QCoreApplication::translate("LuaStaticCheck", "无法创建 Lua 状态机"); return false; }
    luaL_openlibs(L);  // 仅用于识别标准库全局名

    // 1) 仅编译,不执行
    const QByteArray src = script.toUtf8();
    if (luaL_loadstring(L, src.constData()) != LUA_OK) {
        errorMsg = QString::fromUtf8(lua_tostring(L, -1));
        lua_close(L);
        return false;
    }

    // 2) 取编译产物 Proto，扫描全局引用/定义。
    // 公共 API 守卫：loadstring 成功后栈顶必为函数(Lua 闭包)，正常不会触发。
    // 守卫排除非函数后再用 lua_topointer 取地址：5.4 下 lua_topointer 对
    // Lua 闭包返回 LClosure* 地址(版本已由文件顶部 static_assert 锁定)。
    // 不对未判型的值直接调用内部 clLvalue，避免 Debug 下 check_exp 断言。
    if (!lua_isfunction(L, -1)) {
        errorMsg = QCoreApplication::translate("LuaStaticCheck", "编译产物非函数，无法静态检查");
        lua_close(L);
        return false;
    }
    const LClosure* cl = static_cast<const LClosure*>(lua_topointer(L, -1));
    QSet<QString> defined, referenced;
    if (cl && cl->p) scanProto(cl->p, defined, referenced);

    // 3) unknown = referenced - defined - knownFunctions - 标准库(state 非 nil 全局)
    QStringList unknown;
    for (const QString& name : referenced) {
        if (name == QStringLiteral("_ENV")) continue;
        if (defined.contains(name)) continue;
        if (knownFunctions.contains(name)) continue;
        lua_getglobal(L, name.toUtf8().constData());
        const bool isNil = lua_isnil(L, -1);
        lua_pop(L, 1);
        if (!isNil) continue;  // 标准库等全局有定义
        unknown << name;
    }
    lua_close(L);

    if (!unknown.isEmpty()) {
        unknown.sort();
        errorMsg = QCoreApplication::translate("LuaStaticCheck", "未定义的函数/全局: %1").arg(unknown.join(QStringLiteral(", ")));
        return false;
    }
    return true;
}

} // namespace LuaStaticCheck
