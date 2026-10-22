/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */
#include "PlatformBinding.h"
#include <QCoreApplication>
#include "Core/PlatformController.h"
#include "LuaBindingUtil.h"
#include "Lua.hpp"

PlatformBinding::PlatformBinding(PlatformController* controller)
    : m_controller(controller)
{
}

static PlatformBinding* self_from(lua_State* L)
{
    return static_cast<PlatformBinding*>(lua_touserdata(L, lua_upvalueindex(1)));
}

bool PlatformBinding::parseThreeAddr(lua_State* L, int& nX, int& nY, int& nA)
{
    if (!lua_isstring(L, 1)) { luaL_error(L, "Argument #1 (xReg) must be a string (register address)"); return false; }
    const char* xAddr = lua_tostring(L, 1);
    if (!LuaBindingUtil::parseRegisterAddr(xAddr, nX)) { luaL_error(L, "X register address Invalid: %s", xAddr); return false; }
    if (!lua_isstring(L, 2)) { luaL_error(L, "Argument #2 (yReg) must be a string (register address)"); return false; }
    const char* yAddr = lua_tostring(L, 2);
    if (!LuaBindingUtil::parseRegisterAddr(yAddr, nY)) { luaL_error(L, "Y register address Invalid: %s", yAddr); return false; }
    if (!lua_isstring(L, 3)) { luaL_error(L, "Argument #3 (angleReg) must be a string (register address)"); return false; }
    const char* aAddr = lua_tostring(L, 3);
    if (!LuaBindingUtil::parseRegisterAddr(aAddr, nA)) { luaL_error(L, "Angle register address Invalid: %s", aAddr); return false; }
    return true;
}

// description 以 QT_TRANSLATE_NOOP 标记供 lupdate 提取,functions() 中再按当前语言翻译
const PlatformBinding::Fn PlatformBinding::kFns[] = {
    {"MoveAbsInt32",         &PlatformBinding::MoveAbsInt32Wrapper,         "MoveAbsInt32(\"D100\", \"D102\", \"D104\")",         QT_TRANSLATE_NOOP("PlatformBinding", "平台绝对移动,双字")},
    {"MoveAbsFloat",         &PlatformBinding::MoveAbsFloatWrapper,         "MoveAbsFloat(\"D100\", \"D102\", \"D104\")",         QT_TRANSLATE_NOOP("PlatformBinding", "平台绝对移动,浮点数")},
    {"MoveRelativeInt32",    &PlatformBinding::MoveRelativeInt32Wrapper,    "MoveRelativeInt32(\"D100\", \"D102\", \"D104\")",    QT_TRANSLATE_NOOP("PlatformBinding", "平台相对移动,双字")},
    {"MoveRelativeFloat",    &PlatformBinding::MoveRelativeFloatWrapper,    "MoveRelativeFloat(\"D100\", \"D102\", \"D104\")",    QT_TRANSLATE_NOOP("PlatformBinding", "平台相对移动,浮点数")},
    {"WriteCurrentPosInt32", &PlatformBinding::WriteCurrentPosInt32Wrapper, "WriteCurrentPosInt32(\"D100\", \"D102\", \"D104\")", QT_TRANSLATE_NOOP("PlatformBinding", "写入当前位置,双字")},
    {"WriteCurrentPosFloat", &PlatformBinding::WriteCurrentPosFloatWrapper, "WriteCurrentPosFloat(\"D100\", \"D102\", \"D104\")", QT_TRANSLATE_NOOP("PlatformBinding", "写入当前位置,浮点数")},
};

void PlatformBinding::install(lua_State* L)
{
    for (const Fn& f : kFns) {
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, f.fn, 1);
        lua_setglobal(L, f.name);
    }
}

QList<LuaFunctionDoc> PlatformBinding::functions() const
{
    QList<LuaFunctionDoc> docs;
    for (const Fn& f : kFns)
        docs.append({QString::fromUtf8(f.name), QString::fromUtf8(f.snippet),
                     QCoreApplication::translate("PlatformBinding", f.description)});
    return docs;
}

int PlatformBinding::MoveAbsInt32Wrapper(lua_State* L)
{
    PlatformBinding* b = self_from(L);
    int x, y, a;
    if (!parseThreeAddr(L, x, y, a)) return 0;
    b->m_controller->moveAbsolute(x, y, a, PlatformController::NumFormat::Int32);
    return 0;
}

int PlatformBinding::MoveAbsFloatWrapper(lua_State* L)
{
    PlatformBinding* b = self_from(L);
    int x, y, a;
    if (!parseThreeAddr(L, x, y, a)) return 0;
    b->m_controller->moveAbsolute(x, y, a, PlatformController::NumFormat::Float);
    return 0;
}

int PlatformBinding::MoveRelativeInt32Wrapper(lua_State* L)
{
    PlatformBinding* b = self_from(L);
    int x, y, a;
    if (!parseThreeAddr(L, x, y, a)) return 0;
    b->m_controller->moveRelative(x, y, a, PlatformController::NumFormat::Int32);
    return 0;
}

int PlatformBinding::MoveRelativeFloatWrapper(lua_State* L)
{
    PlatformBinding* b = self_from(L);
    int x, y, a;
    if (!parseThreeAddr(L, x, y, a)) return 0;
    b->m_controller->moveRelative(x, y, a, PlatformController::NumFormat::Float);
    return 0;
}

int PlatformBinding::WriteCurrentPosInt32Wrapper(lua_State* L)
{
    PlatformBinding* b = self_from(L);
    int x, y, a;
    if (!parseThreeAddr(L, x, y, a)) return 0;
    b->m_controller->writeCurrentPos(x, y, a, PlatformController::NumFormat::Int32, Platform::Live);
    return 0;
}

int PlatformBinding::WriteCurrentPosFloatWrapper(lua_State* L)
{
    PlatformBinding* b = self_from(L);
    int x, y, a;
    if (!parseThreeAddr(L, x, y, a)) return 0;
    b->m_controller->writeCurrentPos(x, y, a, PlatformController::NumFormat::Float, Platform::Live);
    return 0;
}
