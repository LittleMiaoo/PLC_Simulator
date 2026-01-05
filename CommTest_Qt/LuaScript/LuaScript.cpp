#include "LuaScript.h"


lua_State* LuaScript::g_LuaCompileState = nullptr;
static int nLuaScriptNum = 0;	// 脚本数量
LuaScript* LuaScript::InitialLuaScript(QObject* pParent /*= nullptr*/)
{
	InitialCompileLuaState();
	nLuaScriptNum++;
	
	return new LuaScript(pParent);
}

bool LuaScript::RunLuaScript(const QString& strLuaFile,QString& errorMsg)
{
	QByteArray filePathData = strLuaFile.toLocal8Bit();
	if (luaL_dofile(m_pLua, filePathData.constData()) != LUA_OK)
	{
		const char* error_msg = lua_tostring(m_pLua, -1);
		lua_pop(m_pLua, 1); // 弹出错误信息
		errorMsg = QString::fromUtf8(error_msg);
		return false;
	}

	return true;
}

bool LuaScript::RunLuaScriptWithEditor(const QString &strLuaContent,QString& errorMsg)
{
	QByteArray contentData = strLuaContent.toLocal8Bit();
	if (luaL_dostring(m_pLua, contentData.constData()) != LUA_OK)
	{
		const char* error_msg = lua_tostring(m_pLua, -1);
		lua_pop(m_pLua, 1); // 弹出错误信息

		errorMsg = QString::fromUtf8(error_msg);
		return false;
	}

    return true;
}

LuaScript::~LuaScript()
{
	if (m_pLua)
	{
        lua_close(m_pLua);
		m_pLua = nullptr;
		nLuaScriptNum--;
	}

	// g_LuaCompileState 由 ReleaseCompileLuaState() 统一释放
	// 不在析构函数中自动释放,避免多线程/析构顺序问题
}

LuaScript::LuaScript(QObject* parent /*= nullptr*/)
	:QObject(parent),
	m_pLua(luaL_newstate()),
	m_bLoopValid(false)
{
	luaL_openlibs(m_pLua);

	RegisterLuaFunc();
}

bool LuaScript::RegisterLuaFunc()
{
	lua_pushlightuserdata(m_pLua, this); 
	lua_pushcclosure(m_pLua, SetInt16Wrapper, 1);
	lua_setglobal(m_pLua, "SetInt16");

	lua_pushlightuserdata(m_pLua, this); 
	lua_pushcclosure(m_pLua, SetInt32Wrapper, 1);
	lua_setglobal(m_pLua, "SetInt32");

	lua_pushlightuserdata(m_pLua, this); 
	lua_pushcclosure(m_pLua, SetFloatWrapper, 1);
	lua_setglobal(m_pLua, "SetFloat");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, SetDoubleWrapper, 1);
	lua_setglobal(m_pLua, "SetDouble");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, SetStringWrapper, 1);
	lua_setglobal(m_pLua, "SetString");

	// 注册Get系列函数
	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, GetInt16Wrapper, 1);
	lua_setglobal(m_pLua, "GetInt16");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, GetInt32Wrapper, 1);
	lua_setglobal(m_pLua, "GetInt32");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, GetFloatWrapper, 1);
	lua_setglobal(m_pLua, "GetFloat");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, GetDoubleWrapper, 1);
	lua_setglobal(m_pLua, "GetDouble");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, GetStringWrapper, 1);
	lua_setglobal(m_pLua, "GetString");

	// 注册循环状态函数
	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, IsLoopValidWrapper, 1);
	lua_setglobal(m_pLua, "IsLoopValid");

	//注册sleep
	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, SleepWrapper, 1);
	lua_setglobal(m_pLua, "sleep");

	// 注册平台控制函数
	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, MoveAbsInt32Wrapper, 1);
	lua_setglobal(m_pLua, "MoveAbsInt32");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, MoveAbsFloatWrapper, 1);
	lua_setglobal(m_pLua, "MoveAbsFloat");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, MoveRelativeInt32Wrapper, 1);
	lua_setglobal(m_pLua, "MoveRelativeInt32");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, MoveRelativeFloatWrapper, 1);
	lua_setglobal(m_pLua, "MoveRelativeFloat");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, WriteCurrentPosInt32Wrapper, 1);
	lua_setglobal(m_pLua, "WriteCurrentPosInt32");

	lua_pushlightuserdata(m_pLua, this);
	lua_pushcclosure(m_pLua, WriteCurrentPosFloatWrapper, 1);
	lua_setglobal(m_pLua, "WriteCurrentPosFloat");

	return true;
}

int LuaScript::SetInt16Wrapper(lua_State *L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	//地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}

	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	int value = luaL_checkinteger(L, 2);
	if (value < INT16_MIN || value > INT16_MAX) {
		return luaL_error(L, "Value %d out of int16 range [%d, %d]",
			value, INT16_MIN, INT16_MAX);
	}
    int16_t nsetVal = static_cast<int16_t>(value);

	pThis->SetInt16(nAddr, nsetVal);

	return 0;
}

int LuaScript::SetInt32Wrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	//地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}

	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	int value = luaL_checkinteger(L, 2);
	if (value < INT32_MIN || value > INT32_MAX) {
		return luaL_error(L, "Value %d out of int16 range [%d, %d]",
			value, INT32_MIN, INT32_MAX);
	}
    int32_t nsetVal = static_cast<int32_t>(value);

	pThis->SetInt32(nAddr, nsetVal);

	return 0;
}

int LuaScript::SetFloatWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));
	//地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}
	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}
	
	float fValue = static_cast<float>(luaL_checknumber(L, 2));
	//浮点数越界检查
	if (fValue < -FLT_MAX || fValue > FLT_MAX) {
		return luaL_error(L, "Value %f out of float range [-%f, %f]",
			fValue, -FLT_MAX, FLT_MAX);
	}

	pThis->SetFloat(nAddr, fValue);

	return 0;
}

int LuaScript::SetDoubleWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));
	//地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}
	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;

	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	double dValue = static_cast<double>(luaL_checknumber(L, 2));
	//浮点数越界检查
	if (dValue < -DBL_MAX || dValue > DBL_MAX) {
		return luaL_error(L, "Value %f out of float range [-%f, %f]",
			dValue, -DBL_MAX, DBL_MAX);
	}
	pThis->SetDouble(nAddr, dValue);

	return 0;
}

int LuaScript::SetStringWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));
	//地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}
	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	if (!lua_isstring(L, 2)) {
		return luaL_error(L, "Argument #2 must be a string (register value)");
	}
	const char* strValue = lua_tostring(L, 2);
	
	//字符串长度检查,超过2则截断只保留前两个
	QString strVal = QString::fromUtf8(strValue);
	if (strVal.length() > 2)
	{
		strVal = strVal.left(2);
	}
	pThis->SetString(nAddr, strVal);

	return 0;
}

void LuaScript::SetInt16(int nIndex, int16_t nValue)
{
	if (m_provider) m_provider->SetInt16(nIndex, nValue);
}

void LuaScript::SetInt32(int nIndex, int32_t nValue)
{
	if (m_provider) m_provider->SetInt32(nIndex, nValue);
}

void LuaScript::SetFloat(int nIndex, float fValue)
{
	if (m_provider) m_provider->SetFloat(nIndex, fValue);
}

void LuaScript::SetDouble(int nIndex, double dValue)
{
	if (m_provider) m_provider->SetDouble(nIndex, dValue);
}

void LuaScript::SetString(int nIndex, QString strValue)
{
	if (m_provider) m_provider->SetString(nIndex, strValue);
}

// ===== Get系列函数实现 =====
int LuaScript::GetInt16Wrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}

	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	int16_t value = pThis->GetInt16(nAddr);
	lua_pushinteger(L, value);
	return 1;
}

int LuaScript::GetInt32Wrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}

	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	int32_t value = pThis->GetInt32(nAddr);
	lua_pushinteger(L, value);
	return 1;
}

int LuaScript::GetFloatWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}

	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	float value = pThis->GetFloat(nAddr);
	lua_pushnumber(L, value);
	return 1;
}

int LuaScript::GetDoubleWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}

	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	double value = pThis->GetDouble(nAddr);
	lua_pushnumber(L, value);
	return 1;
}

int LuaScript::GetStringWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 地址获取
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 must be a string (register address)");
	}

	const char* addr = lua_tostring(L, 1);
	int nAddr = 0;
	if (LuaScript::ParseRegisterAddr(addr, nAddr) == false)
	{
		return luaL_error(L, "Register address Invalid: %s", addr);
	}

	QString value = pThis->GetString(nAddr);
	QByteArray utf8Data = value.toUtf8();
	lua_pushstring(L, utf8Data.constData());
	return 1;
}

int16_t LuaScript::GetInt16(int nIndex)
{
    return m_provider ? m_provider->GetInt16(nIndex) : int16_t(0);
}

int32_t LuaScript::GetInt32(int nIndex)
{
    return m_provider ? m_provider->GetInt32(nIndex) : int32_t(0);
}

float LuaScript::GetFloat(int nIndex)
{
    return m_provider ? m_provider->GetFloat(nIndex) : 0.0f;
}

double LuaScript::GetDouble(int nIndex)
{
    return m_provider ? m_provider->GetDouble(nIndex) : 0.0;
}

QString LuaScript::GetString(int nIndex)
{
    return m_provider ? m_provider->GetString(nIndex) : QString();
}

// ===== 循环状态函数实现 =====
int LuaScript::IsLoopValidWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	bool bValid = pThis->GetLoopValid();
	lua_pushboolean(L, bValid);
	return 1;
}

int LuaScript::SleepWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 获取参数
	if (!lua_isnumber(L, 1)) {
		return luaL_error(L, "Argument #1 (milliseconds) must be a number");
	}
	int milliseconds = lua_tointeger(L, 1);

	/*QThread::msleep(milliseconds);*/
	QEventLoop loop;
	QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
	loop.exec();
	return 0;
}


// ===== 平台控制函数实现 =====
int LuaScript::MoveAbsInt32Wrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 解析X地址
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 (xReg) must be a string (register address)");
	}
	const char* xAddr = lua_tostring(L, 1);
	int nXAddr = 0;
	if (!LuaScript::ParseRegisterAddr(xAddr, nXAddr)) {
		return luaL_error(L, "X register address Invalid: %s", xAddr);
	}

	// 解析Y地址
	if (!lua_isstring(L, 2)) {
		return luaL_error(L, "Argument #2 (yReg) must be a string (register address)");
	}
	const char* yAddr = lua_tostring(L, 2);
	int nYAddr = 0;
	if (!LuaScript::ParseRegisterAddr(yAddr, nYAddr)) {
		return luaL_error(L, "Y register address Invalid: %s", yAddr);
	}

	// 解析角度地址
	if (!lua_isstring(L, 3)) {
		return luaL_error(L, "Argument #3 (angleReg) must be a string (register address)");
	}
	const char* angleAddr = lua_tostring(L, 3);
	int nAngleAddr = 0;
	if (!LuaScript::ParseRegisterAddr(angleAddr, nAngleAddr)) {
		return luaL_error(L, "Angle register address Invalid: %s", angleAddr);
	}

	pThis->MoveAbsInt32(nXAddr, nYAddr, nAngleAddr);
	return 0;
}

int LuaScript::MoveAbsFloatWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 解析X地址
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 (xReg) must be a string (register address)");
	}
	const char* xAddr = lua_tostring(L, 1);
	int nXAddr = 0;
	if (!LuaScript::ParseRegisterAddr(xAddr, nXAddr)) {
		return luaL_error(L, "X register address Invalid: %s", xAddr);
	}

	// 解析Y地址
	if (!lua_isstring(L, 2)) {
		return luaL_error(L, "Argument #2 (yReg) must be a string (register address)");
	}
	const char* yAddr = lua_tostring(L, 2);
	int nYAddr = 0;
	if (!LuaScript::ParseRegisterAddr(yAddr, nYAddr)) {
		return luaL_error(L, "Y register address Invalid: %s", yAddr);
	}

	// 解析角度地址
	if (!lua_isstring(L, 3)) {
		return luaL_error(L, "Argument #3 (angleReg) must be a string (register address)");
	}
	const char* angleAddr = lua_tostring(L, 3);
	int nAngleAddr = 0;
	if (!LuaScript::ParseRegisterAddr(angleAddr, nAngleAddr)) {
		return luaL_error(L, "Angle register address Invalid: %s", angleAddr);
	}

	pThis->MoveAbsFloat(nXAddr, nYAddr, nAngleAddr);
	return 0;
}

int LuaScript::MoveRelativeInt32Wrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 解析X地址
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 (xReg) must be a string (register address)");
	}
	const char* xAddr = lua_tostring(L, 1);
	int nXAddr = 0;
	if (!LuaScript::ParseRegisterAddr(xAddr, nXAddr)) {
		return luaL_error(L, "X register address Invalid: %s", xAddr);
	}

	// 解析Y地址
	if (!lua_isstring(L, 2)) {
		return luaL_error(L, "Argument #2 (yReg) must be a string (register address)");
	}
	const char* yAddr = lua_tostring(L, 2);
	int nYAddr = 0;
	if (!LuaScript::ParseRegisterAddr(yAddr, nYAddr)) {
		return luaL_error(L, "Y register address Invalid: %s", yAddr);
	}

	// 解析角度地址
	if (!lua_isstring(L, 3)) {
		return luaL_error(L, "Argument #3 (angleReg) must be a string (register address)");
	}
	const char* angleAddr = lua_tostring(L, 3);
	int nAngleAddr = 0;
	if (!LuaScript::ParseRegisterAddr(angleAddr, nAngleAddr)) {
		return luaL_error(L, "Angle register address Invalid: %s", angleAddr);
	}

	pThis->MoveRelativeInt32(nXAddr, nYAddr, nAngleAddr);
	return 0;
}

int LuaScript::MoveRelativeFloatWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 解析X地址
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 (xReg) must be a string (register address)");
	}
	const char* xAddr = lua_tostring(L, 1);
	int nXAddr = 0;
	if (!LuaScript::ParseRegisterAddr(xAddr, nXAddr)) {
		return luaL_error(L, "X register address Invalid: %s", xAddr);
	}

	// 解析Y地址
	if (!lua_isstring(L, 2)) {
		return luaL_error(L, "Argument #2 (yReg) must be a string (register address)");
	}
	const char* yAddr = lua_tostring(L, 2);
	int nYAddr = 0;
	if (!LuaScript::ParseRegisterAddr(yAddr, nYAddr)) {
		return luaL_error(L, "Y register address Invalid: %s", yAddr);
	}

	// 解析角度地址
	if (!lua_isstring(L, 3)) {
		return luaL_error(L, "Argument #3 (angleReg) must be a string (register address)");
	}
	const char* angleAddr = lua_tostring(L, 3);
	int nAngleAddr = 0;
	if (!LuaScript::ParseRegisterAddr(angleAddr, nAngleAddr)) {
		return luaL_error(L, "Angle register address Invalid: %s", angleAddr);
	}

	pThis->MoveRelativeFloat(nXAddr, nYAddr, nAngleAddr);
	return 0;
}

int LuaScript::WriteCurrentPosInt32Wrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 解析X地址
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 (xReg) must be a string (register address)");
	}
	const char* xAddr = lua_tostring(L, 1);
	int nXAddr = 0;
	if (!LuaScript::ParseRegisterAddr(xAddr, nXAddr)) {
		return luaL_error(L, "X register address Invalid: %s", xAddr);
	}

	// 解析Y地址
	if (!lua_isstring(L, 2)) {
		return luaL_error(L, "Argument #2 (yReg) must be a string (register address)");
	}
	const char* yAddr = lua_tostring(L, 2);
	int nYAddr = 0;
	if (!LuaScript::ParseRegisterAddr(yAddr, nYAddr)) {
		return luaL_error(L, "Y register address Invalid: %s", yAddr);
	}

	// 解析角度地址
	if (!lua_isstring(L, 3)) {
		return luaL_error(L, "Argument #3 (angleReg) must be a string (register address)");
	}
	const char* angleAddr = lua_tostring(L, 3);
	int nAngleAddr = 0;
	if (!LuaScript::ParseRegisterAddr(angleAddr, nAngleAddr)) {
		return luaL_error(L, "Angle register address Invalid: %s", angleAddr);
	}

	pThis->WriteCurrentPosInt32(nXAddr, nYAddr, nAngleAddr);
	return 0;
}

int LuaScript::WriteCurrentPosFloatWrapper(lua_State* L)
{
	LuaScript* pThis = static_cast<LuaScript*>(lua_touserdata(L, lua_upvalueindex(1)));

	// 解析X地址
	if (!lua_isstring(L, 1)) {
		return luaL_error(L, "Argument #1 (xReg) must be a string (register address)");
	}
	const char* xAddr = lua_tostring(L, 1);
	int nXAddr = 0;
	if (!LuaScript::ParseRegisterAddr(xAddr, nXAddr)) {
		return luaL_error(L, "X register address Invalid: %s", xAddr);
	}

	// 解析Y地址
	if (!lua_isstring(L, 2)) {
		return luaL_error(L, "Argument #2 (yReg) must be a string (register address)");
	}
	const char* yAddr = lua_tostring(L, 2);
	int nYAddr = 0;
	if (!LuaScript::ParseRegisterAddr(yAddr, nYAddr)) {
		return luaL_error(L, "Y register address Invalid: %s", yAddr);
	}

	// 解析角度地址
	if (!lua_isstring(L, 3)) {
		return luaL_error(L, "Argument #3 (angleReg) must be a string (register address)");
	}
	const char* angleAddr = lua_tostring(L, 3);
	int nAngleAddr = 0;
	if (!LuaScript::ParseRegisterAddr(angleAddr, nAngleAddr)) {
		return luaL_error(L, "Angle register address Invalid: %s", angleAddr);
	}

	pThis->WriteCurrentPosFloat(nXAddr, nYAddr, nAngleAddr);
	return 0;
}


void LuaScript::MoveAbsInt32(int nXIndex, int nYIndex, int nAngleIndex)
{
	// double x = GetInt32(nXIndex);
	// double y = GetInt32(nYIndex);
	// double angle = GetInt32(nAngleIndex);
	// emit MovePlatformAbsInt32(x, y, angle);
	if (m_provider) m_provider->MovePlatformAbsInt32(nXIndex, nYIndex, nAngleIndex);
}

void LuaScript::MoveAbsFloat(int nXIndex, int nYIndex, int nAngleIndex)
{
	// double x = static_cast<double>(GetFloat(nXIndex));
	// double y = static_cast<double>(GetFloat(nYIndex));
	// double angle = static_cast<double>(GetFloat(nAngleIndex));
	// emit MovePlatformAbsFloat(x, y, angle);
	if (m_provider) m_provider->MovePlatformAbsFloat(nXIndex, nYIndex, nAngleIndex);
}

void LuaScript::MoveRelativeInt32(int nXIndex, int nYIndex, int nAngleIndex)
{
	// double x = GetInt32(nXIndex);
	// double y = GetInt32(nYIndex);
	// double angle = GetInt32(nAngleIndex);
	// emit MovePlatformRelativeInt32(x, y, angle);
	if (m_provider) m_provider->MovePlatformRelativeInt32(nXIndex, nYIndex, nAngleIndex);
}

void LuaScript::MoveRelativeFloat(int nXIndex, int nYIndex, int nAngleIndex)
{
	// double x = static_cast<double>(GetFloat(nXIndex));
	// double y = static_cast<double>(GetFloat(nYIndex));
	// double angle = static_cast<double>(GetFloat(nAngleIndex));
	// emit MovePlatformRelativeFloat(x, y, angle);
	if (m_provider) m_provider->MovePlatformRelativeFloat(nXIndex, nYIndex, nAngleIndex);
}

void LuaScript::WriteCurrentPosInt32(int nXIndex, int nYIndex, int nAngleIndex)
{
	// double x = GetInt32(nXIndex);
	// double y = GetInt32(nYIndex);
	// double angle = GetInt32(nAngleIndex);
	// emit WriteCurrentPosInt32(x, y, angle);
	if (m_provider) m_provider->WriteCurrentPosInt32(nXIndex, nYIndex, nAngleIndex);
}

void LuaScript::WriteCurrentPosFloat(int nXIndex, int nYIndex, int nAngleIndex)
{
	// double x = static_cast<double>(GetFloat(nXIndex));
	// double y = static_cast<double>(GetFloat(nYIndex));
	// double angle = static_cast<double>(GetFloat(nAngleIndex));
	// emit WriteCurrentPosFloat(x, y, angle);
	if (m_provider) m_provider->WriteCurrentPosFloat(nXIndex, nYIndex, nAngleIndex);
}
bool LuaScript::ParseRegisterAddr(const char* strAddr, int& nAddr, int nMinVal /*= 0*/, int nMaxVal /*= 100000*/)
{
	QString addressStr = QString::fromUtf8(strAddr).trimmed();

	// 检查长度
	if (addressStr.length() < 2 || addressStr.length() > 10) {
		return false;
	}

	// 检查第一个字符
	if (addressStr[0] != 'D') {
		return false;
	}

	// 提取数字部分
	QString numberStr = addressStr.mid(1);

	// 检查是否全是数字
	for (QChar ch : numberStr) {
		if (!ch.isDigit()) {
			return false;
		}
	}

	// 转换为数字并检查范围
	bool ok;
	nAddr = numberStr.toInt(&ok);

	if (!ok || nAddr < nMinVal || nAddr > nMaxVal) {
		return false;
	}

	return true;
}

// 静态C函数用于编译检查
static int DummyLuaFunc(lua_State* L) {
	return 0;
}

static int DummyLuaFuncReturn1(lua_State* L) {
	lua_pushinteger(L, 0);
	return 1;
}

void LuaScript::InitialCompileLuaState()
{
	static std::once_flag initFlag;

	auto onceFunc = [](){
	//检查全局静态状态机是否为空
	if (g_LuaCompileState == nullptr)
	{
		g_LuaCompileState = luaL_newstate();
		if (g_LuaCompileState == nullptr)
		{
			return;
		}
		luaL_openlibs(g_LuaCompileState);

		//注册Set系列函数
		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "SetInt16");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "SetInt32");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "SetFloat");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "SetDouble");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "SetString");

		//注册Get系列函数
		lua_pushcfunction(g_LuaCompileState, DummyLuaFuncReturn1);
		lua_setglobal(g_LuaCompileState, "GetInt16");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFuncReturn1);
		lua_setglobal(g_LuaCompileState, "GetInt32");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFuncReturn1);
		lua_setglobal(g_LuaCompileState, "GetFloat");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFuncReturn1);
		lua_setglobal(g_LuaCompileState, "GetDouble");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFuncReturn1);
		lua_setglobal(g_LuaCompileState, "GetString");

		//注册循环状态函数
		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "IsLoopValid");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "sleep");
		//注册平台控制函数
		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "MoveAbsInt32");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "MoveAbsFloat");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "MoveRelativeInt32");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "MoveRelativeFloat");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "WriteCurrentPosInt32");

		lua_pushcfunction(g_LuaCompileState, DummyLuaFunc);
		lua_setglobal(g_LuaCompileState, "WriteCurrentPosFloat");
		 //qDebug() << "Compilation Lua state initialized";
	}};

	std::call_once(initFlag, onceFunc);
}

void LuaScript::ReleaseCompileLuaState()
{
	if (g_LuaCompileState != nullptr)
	{
		lua_close(g_LuaCompileState);
		g_LuaCompileState = nullptr;
	}
}

bool LuaScript::CheckLuaScript(const QString &strLuaFile,QString& strErrorInfo)
{
    if (g_LuaCompileState == nullptr)	return false;

	//if (strLuaFile.isEmpty()) return false;

	QByteArray scriptData = strLuaFile.toLatin1();
	if(luaL_dostring(g_LuaCompileState, scriptData.data()) != LUA_OK)
	{
		const char* error_msg = lua_tostring(g_LuaCompileState, -1);
		lua_pop(g_LuaCompileState, 1);

		strErrorInfo = QString::fromUtf8(error_msg);

		return false;
	}

	return true;

}
