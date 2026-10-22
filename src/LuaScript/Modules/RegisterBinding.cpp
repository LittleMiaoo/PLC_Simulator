/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */
#include "RegisterBinding.h"
#include <QCoreApplication>
#include "Core/RegisterStore.h"
#include "LuaBindingUtil.h"
#include "Lua.hpp"

#include <QString>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>

RegisterBinding::RegisterBinding(RegisterStore* store)
    : m_store(store)
{
}

static RegisterBinding* self_from(lua_State* L)
{
    return static_cast<RegisterBinding*>(lua_touserdata(L, lua_upvalueindex(1)));
}

// 各 wrapper 共用前导:取 binding、校验 arg1 为地址字符串、解析出 nAddr。
// 出错时 luaL_error 直接 longjmp,不返回。
static RegisterBinding* prologue(lua_State* L, int& nAddr)
{
    RegisterBinding* b = self_from(L);
    if (!lua_isstring(L, 1)) luaL_error(L, "Argument #1 must be a string (register address)");
    const char* addr = lua_tostring(L, 1);
    if (!LuaBindingUtil::parseRegisterAddr(addr, nAddr)) luaL_error(L, "Register address Invalid: %s", addr);
    return b;
}

// description 以 QT_TRANSLATE_NOOP 标记供 lupdate 提取,functions() 中再按当前语言翻译
const RegisterBinding::Fn RegisterBinding::kFns[] = {
    {"SetInt16",  &RegisterBinding::SetInt16Wrapper,  "SetInt16(\"D100\", 123)",      QT_TRANSLATE_NOOP("RegisterBinding", "设置寄存器值,单字")},
    {"SetInt32",  &RegisterBinding::SetInt32Wrapper,  "SetInt32(\"D100\", 123)",      QT_TRANSLATE_NOOP("RegisterBinding", "设置寄存器值,双字")},
    {"SetFloat",  &RegisterBinding::SetFloatWrapper,  "SetFloat(\"D100\", 123.45)",   QT_TRANSLATE_NOOP("RegisterBinding", "设置寄存器值,浮点数")},
    {"SetDouble", &RegisterBinding::SetDoubleWrapper, "SetDouble(\"D100\", 123.45)",  QT_TRANSLATE_NOOP("RegisterBinding", "设置寄存器值,双精度浮点数")},
    {"SetString", &RegisterBinding::SetStringWrapper, "SetString(\"D100\", \"AB\")",  QT_TRANSLATE_NOOP("RegisterBinding", "设置寄存器字符串")},
    {"GetInt16",  &RegisterBinding::GetInt16Wrapper,  "GetInt16(\"D100\")",           QT_TRANSLATE_NOOP("RegisterBinding", "读取寄存器值,单字")},
    {"GetInt32",  &RegisterBinding::GetInt32Wrapper,  "GetInt32(\"D100\")",           QT_TRANSLATE_NOOP("RegisterBinding", "读取寄存器值,双字")},
    {"GetFloat",  &RegisterBinding::GetFloatWrapper,  "GetFloat(\"D100\")",           QT_TRANSLATE_NOOP("RegisterBinding", "读取寄存器值,浮点数")},
    {"GetDouble", &RegisterBinding::GetDoubleWrapper, "GetDouble(\"D100\")",          QT_TRANSLATE_NOOP("RegisterBinding", "读取寄存器值,双精度浮点数")},
    {"GetString", &RegisterBinding::GetStringWrapper, "GetString(\"D100\")",          QT_TRANSLATE_NOOP("RegisterBinding", "读取寄存器字符串")},
};

void RegisterBinding::install(lua_State* L)
{
    for (const Fn& f : kFns) {
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, f.fn, 1);
        lua_setglobal(L, f.name);
    }
}

QList<LuaFunctionDoc> RegisterBinding::functions() const
{
    QList<LuaFunctionDoc> docs;
    for (const Fn& f : kFns)
        docs.append({QString::fromUtf8(f.name), QString::fromUtf8(f.snippet),
                     QCoreApplication::translate("RegisterBinding", f.description)});
    return docs;
}

int RegisterBinding::SetInt16Wrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    lua_Integer value = luaL_checkinteger(L, 2);
    if (value < INT16_MIN || value > INT16_MAX) {
        char msg[128];
        std::snprintf(msg, sizeof(msg), "Value %lld out of int16 range [%d, %d]",
                      static_cast<long long>(value), INT16_MIN, INT16_MAX);
        return luaL_error(L, "%s", msg);
    }
    b->m_store->SetInt16(nAddr, static_cast<int16_t>(value));
    return 0;
}

int RegisterBinding::SetInt32Wrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    lua_Integer value = luaL_checkinteger(L, 2);
    if (value < INT32_MIN || value > INT32_MAX) {
        char msg[128];
        std::snprintf(msg, sizeof(msg), "Value %lld out of int32 range [%d, %d]",
                      static_cast<long long>(value), INT32_MIN, INT32_MAX);
        return luaL_error(L, "%s", msg);
    }
    b->m_store->SetInt32(nAddr, static_cast<int32_t>(value));
    return 0;
}

int RegisterBinding::SetFloatWrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    // 在转 float 前以 double 域判定:拦截 inf/nan 与超出 float 表示范围的入参
    double dValue = static_cast<double>(luaL_checknumber(L, 2));
    if (!std::isfinite(dValue) || std::fabs(dValue) > static_cast<double>(FLT_MAX))
        return luaL_error(L, "Value %f out of float range [-%f, %f]",
                          dValue, static_cast<double>(FLT_MAX), static_cast<double>(FLT_MAX));
    b->m_store->SetFloat(nAddr, static_cast<float>(dValue));
    return 0;
}

int RegisterBinding::SetDoubleWrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    // lua_Number 即 double,有限值不可能越 double 界;此处只需拦截 inf/nan
    double dValue = static_cast<double>(luaL_checknumber(L, 2));
    if (!std::isfinite(dValue))
        return luaL_error(L, "Value %f is not a finite double (inf/nan)", dValue);
    b->m_store->SetDouble(nAddr, dValue);
    return 0;
}

int RegisterBinding::SetStringWrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    if (!lua_isstring(L, 2)) return luaL_error(L, "Argument #2 must be a string (register value)");
    const char* strValue = lua_tostring(L, 2);
    QString strVal = QString::fromUtf8(strValue);
    if (strVal.length() > 2) strVal = strVal.left(2);
    b->m_store->SetString(nAddr, strVal);
    return 0;
}

int RegisterBinding::GetInt16Wrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    lua_pushinteger(L, b->m_store->GetInt16(nAddr));
    return 1;
}

int RegisterBinding::GetInt32Wrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    lua_pushinteger(L, b->m_store->GetInt32(nAddr));
    return 1;
}

int RegisterBinding::GetFloatWrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    lua_pushnumber(L, b->m_store->GetFloat(nAddr));
    return 1;
}

int RegisterBinding::GetDoubleWrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    lua_pushnumber(L, b->m_store->GetDouble(nAddr));
    return 1;
}

int RegisterBinding::GetStringWrapper(lua_State* L)
{
    int nAddr = 0;
    RegisterBinding* b = prologue(L, nAddr);
    QString value = b->m_store->GetString(nAddr);
    QByteArray utf8 = value.toUtf8();
    lua_pushstring(L, utf8.constData());
    return 1;
}
