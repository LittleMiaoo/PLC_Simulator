#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#include <QObject>
#include "Lua.hpp"

class LuaScript :public QObject
{
	Q_OBJECT
public:
	//构造函数
    ~LuaScript() ;

	//禁用拷贝构造&赋值构造
    LuaScript(const LuaScript&) = delete;
    LuaScript& operator=(const LuaScript&) = delete;

    static LuaScript* InitialLuaScript(QObject* pParent = nullptr);

	//设置循环是否有效
	void SetLoopValid(bool bValid) {
		m_bLoopValid = bValid;
	}

	bool RunLuaScript(const QString& strLuaFile,QString& errorMsg);
	bool RunLuaScriptWithEditor(const QString& strLuaContent,QString& errorMsg);

private:
	//构造函数
	LuaScript(QObject* parent = nullptr);

	lua_State* m_pLua;

	

	bool m_bLoopValid;	//循环是否有效

	bool RegisterLuaFunc();

private:
	//注册到Lua的各种数据类型Set静态封装函数
    static int SetInt16Wrapper(lua_State* L);
	static int SetInt32Wrapper(lua_State* L);
	static int SetFloatWrapper(lua_State* L);
	static int SetDoubleWrapper(lua_State* L);
    static int SetStringWrapper(lua_State* L);

	//各种数据类型的实际执行函数
	void SetInt16(int nIndex, int16_t nValue);
	void SetInt32(int nIndex, int32_t nValue);
	void SetFloat(int nIndex, float fValue);
	void SetDouble(int nIndex, double dValue);
	void SetString(int nIndex, QString strValue);

	//验证Lua传入的地址字符串是否有效并解析成地址
	static bool ParseRegisterAddr(const char* strAddr, int& nAddr,int nMinVal = 0,int nMaxVal = 100000);

private:
	static lua_State* g_LuaCompileState;	//脚本编译检查状态机
	static void InitialCompileLuaState();	//初始化脚本编译检查状态机

public:
	static bool CheckLuaScript(const QString& strLuaFile,QString& strErrorInfo);	//检查脚本是否正常
	static QStringList getRegisteredFunctions() {
        // 这应该返回实际注册到Lua状态机的函数列表
        return QStringList() << "SetInt16" << "SetInt32" << "SetFloat" << "SetDouble" << "SetString";
    }
    
    // static QString getFunctionTemplate(const QString &functionName) {
    //     // 返回函数的模板，包含默认参数
    //     if (functionName == "SetInt16") {
    //         return "SetInt16(\"D100\", 123)";
    //     } else if (functionName == "GetInt16") {
    //         return "GetInt16(\"D100\")";
    //     } else if (functionName == "SetFloat") {
    //         return "SetFloat(\"F100\", 123.45)";
    //     } else if (functionName == "GetFloat") {
    //         return "GetFloat(\"F100\")";
    //     }
    //     return functionName + "()";
    // }

signals:
	void SetRegisterValInt16(int nIndex, int16_t nValue);
	void SetRegisterValInt32(int nIndex, int32_t nValue);
    void SetRegisterValFloat(int nIndex, float fValue);
    void SetRegisterValDouble(int nIndex, double dValue);
    void SetRegisterValString(int nIndex, QString strValue);
	

};

#endif	// LUASCRIPT_H


