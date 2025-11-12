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
	
	if (luaL_dofile(m_pLua, strLuaFile.toLocal8Bit().constData()) != LUA_OK)
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
	if (luaL_dostring(m_pLua, strLuaContent.toLocal8Bit().constData()) != LUA_OK)
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
		nLuaScriptNum--;
	}

	if (nLuaScriptNum == 0 && g_LuaCompileState)
	{
		lua_close(g_LuaCompileState);
		g_LuaCompileState = nullptr;
	}
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
	emit SetRegisterValInt16(nIndex, nValue);
}

void LuaScript::SetInt32(int nIndex, int32_t nValue)
{
	emit SetRegisterValInt32(nIndex, nValue);
}

void LuaScript::SetFloat(int nIndex, float fValue)
{
	emit SetRegisterValFloat(nIndex, fValue);
}

void LuaScript::SetDouble(int nIndex, double dValue)
{
	emit SetRegisterValDouble(nIndex, dValue);
}

void LuaScript::SetString(int nIndex, QString strValue)
{
	emit SetRegisterValString(nIndex, strValue);
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

		auto LamdaFunc =  [](lua_State* L) -> int {
				//qDebug() << "[Check] Function called during syntax check";
					return 0;};
		//注册函数
		lua_pushcfunction(g_LuaCompileState,LamdaFunc);
		lua_setglobal(g_LuaCompileState, "SetInt16");

		lua_pushcfunction(g_LuaCompileState,LamdaFunc);
		lua_setglobal(g_LuaCompileState, "SetInt32");

		lua_pushcfunction(g_LuaCompileState,LamdaFunc);
		lua_setglobal(g_LuaCompileState, "SetFloat");

		lua_pushcfunction(g_LuaCompileState,LamdaFunc);
		lua_setglobal(g_LuaCompileState, "SetDouble");

		lua_pushcfunction(g_LuaCompileState,LamdaFunc);
		lua_setglobal(g_LuaCompileState, "SetString");

		 //qDebug() << "Compilation Lua state initialized";
	}};

	std::call_once(initFlag, onceFunc);
}

bool LuaScript::CheckLuaScript(const QString &strLuaFile,QString& strErrorInfo)
{
    if (g_LuaCompileState == nullptr)	return false;

	//if (strLuaFile.isEmpty()) return false;

	if(luaL_dostring(g_LuaCompileState, strLuaFile.toLatin1().data()) != LUA_OK)
	{
		const char* error_msg = lua_tostring(g_LuaCompileState, -1);
		lua_pop(g_LuaCompileState, 1);

		strErrorInfo = QString::fromUtf8(error_msg);

		return false;
	}

	return true;
	
}
