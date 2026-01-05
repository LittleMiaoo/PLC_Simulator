/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao <mao.wang.dev@foxmail.com>
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
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
    class IDataProvider {
    public:
        virtual ~IDataProvider() = default;
        virtual int16_t GetInt16(int index) = 0;
        virtual int32_t GetInt32(int index) = 0;
        virtual float GetFloat(int index) = 0;
        virtual double GetDouble(int index) = 0;
        virtual QString GetString(int index) = 0;
        virtual void SetInt16(int index, int16_t value) = 0;
        virtual void SetInt32(int index, int32_t value) = 0;
        virtual void SetFloat(int index, float value) = 0;
        virtual void SetDouble(int index, double value) = 0;
        virtual void SetString(int index, const QString& value) = 0;

		//控制平台移动,传入参数为地址
		virtual void MovePlatformAbsInt32(int Xaddr, int Yaddr, int Angleaddr)= 0;
		virtual void MovePlatformAbsFloat(int Xaddr, int Yaddr, int Angleaddr) = 0;
		virtual void MovePlatformRelativeInt32(int Xaddr, int Yaddr, int Angleaddr) = 0;
		virtual void MovePlatformRelativeFloat(int Xaddr, int Yaddr, int Angleaddr) = 0;
		//写入当前轴位置,传入参数为地址
		virtual void WriteCurrentPosInt32(int Xaddr, int Yaddr, int Angleaddr) = 0;
		virtual void WriteCurrentPosFloat(int Xaddr, int Yaddr, int Angleaddr) = 0;
    };
    void SetDataProvider(IDataProvider* provider) { m_provider = provider; }

	//设置循环是否有效
	void SetLoopValid(bool bValid) {
		m_bLoopValid = bValid;
	}

	// 获取循环是否有效
	bool GetLoopValid() const {
		return m_bLoopValid;
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

	//注册到Lua的各种数据类型Get静态封装函数
	static int GetInt16Wrapper(lua_State* L);
	static int GetInt32Wrapper(lua_State* L);
	static int GetFloatWrapper(lua_State* L);
	static int GetDoubleWrapper(lua_State* L);
	static int GetStringWrapper(lua_State* L);

	//注册到Lua的循环状态判断函数
	static int IsLoopValidWrapper(lua_State* L);
	static int SleepWrapper(lua_State* L);

	//注册到Lua的平台控制函数
	static int MoveAbsInt32Wrapper(lua_State* L);
	static int MoveAbsFloatWrapper(lua_State* L);
	static int MoveRelativeInt32Wrapper(lua_State* L);
	static int MoveRelativeFloatWrapper(lua_State* L);

	//注册到Lua的平台数据写入函数
	static int WriteCurrentPosInt32Wrapper(lua_State* L);
	static int WriteCurrentPosFloatWrapper(lua_State* L);

	//各种数据类型的实际执行函数
	void SetInt16(int nIndex, int16_t nValue);
	void SetInt32(int nIndex, int32_t nValue);
	void SetFloat(int nIndex, float fValue);
	void SetDouble(int nIndex, double dValue);
	void SetString(int nIndex, QString strValue);

	//获取寄存器数据函数
	int16_t GetInt16(int nIndex);
	int32_t GetInt32(int nIndex);
	float GetFloat(int nIndex);
	double GetDouble(int nIndex);
	QString GetString(int nIndex);

	//平台控制函数实际执行
	void MoveAbsInt32(int nXIndex, int nYIndex, int nAngleIndex);
	void MoveAbsFloat(int nXIndex, int nYIndex, int nAngleIndex);
	void MoveRelativeInt32(int nXIndex, int nYIndex, int nAngleIndex);
	void MoveRelativeFloat(int nXIndex, int nYIndex, int nAngleIndex);

	//平台数据写入函数实际执行
	void WriteCurrentPosInt32(int nXIndex, int nYIndex, int nAngleIndex);
	void WriteCurrentPosFloat(int nXIndex, int nYIndex, int nAngleIndex);

	//验证Lua传入的地址字符串是否有效并解析成地址
	static bool ParseRegisterAddr(const char* strAddr, int& nAddr,int nMinVal = 0,int nMaxVal = 100000);

private:
	static lua_State* g_LuaCompileState;	//脚本编译检查状态机
	static void InitialCompileLuaState();	//初始化脚本编译检查状态机

public:
	static void ReleaseCompileLuaState();	//释放脚本编译检查状态机
	static bool CheckLuaScript(const QString& strLuaFile,QString& strErrorInfo);	//检查脚本是否正常
	static QStringList getRegisteredFunctions() {
        // 这应该返回实际注册到Lua状态机的函数列表
        return QStringList() << "SetInt16" << "SetInt32" << "SetFloat" << "SetDouble" << "SetString"
                            << "GetInt16" << "GetInt32" << "GetFloat" << "GetDouble" << "GetString"
                            << "IsLoopValid"<< "sleep"
                            << "MoveAbsInt32" << "MoveAbsFloat" << "MoveRelativeInt32" << "MoveRelativeFloat"
							<< "WriteCurrentPosInt32" << "WriteCurrentPosFloat";
    }


// signals:
// 	// 平台控制信号
// 	void MovePlatformAbsInt32(int32_t nX, int32_t nY, int32_t nAngle);
// 	void MovePlatformAbsFloat(double dX, double dY, double dAngle);
// 	void MovePlatformRelativeInt32(int32_t nX, int32_t nY, int32_t nAngle);
//     void MovePlatformRelativeFloat(double dX, double dY, double dAngle);
// 	// 平台数据写入信号
// 	void WriteCurrentPosInt32(int32_t nX, int32_t nY, int32_t nAngle);
// 	void WriteCurrentPosFloat(double dX, double dY, double dAngle);

private:
    IDataProvider* m_provider = nullptr;
};

#endif	// LUASCRIPT_H


