/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao <mao.wang.dev@foxmail.com>
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef COMM_PROTOCOL_MITSUBISHIQBINARY_H
#define COMM_PROTOCOL_MITSUBISHIQBINARY_H
#include "CommProtocolBase.h"
class CommProMitsubishiQBinary :
	public CommProtocolBase
{
	Q_OBJECT
public:
	explicit CommProMitsubishiQBinary(QObject* pParent);
	
	CommProMitsubishiQBinary(const CommProMitsubishiQBinary& other) = delete;
	CommProMitsubishiQBinary& operator= (const CommProMitsubishiQBinary& other) = delete;

	virtual ~CommProMitsubishiQBinary() = default;
	
	//基类的抽象接口重写
public:
	

	virtual bool AnalyzeCmdInfo(QByteArray strInfo, CmdType& cCmdType) override;

	//20251101	wm	解析读寄存器指令
	virtual bool AnalyzeReadReg(QByteArray strInfo, long& nRegAddr, int& nWriteNum) override;

	//20251101	wm	打包回复读寄存器指令信息
	virtual bool PackReportReadRegInfo(QByteArray& strInfo, long nRegAddr, int nWriteNum, const std::vector<int16_t>& vWriteData) override;

	//20251101	wm	解析写寄存器指令
	virtual bool AnalyzeWriteReg(QByteArray strInfo, long& nRegAddr, int& nWriteNum, std::vector<int16_t>& vWriteData) override;

	//20251101	wm	打包回复写寄存器指令信息
	virtual bool PackReportWriteRegInfo(QByteArray& strInfo) override;

private:

	//************************************
	// Method:    CheckCmdInfoValid
	// FullName:  CommProMitsubishiQBinary::CheckCmdInfoValid
	// Access:    private 
	// Returns:   bool
	// Qualifier:
	// Parameter: QByteArray & strInfo : Input:完整的指令信息；output:寄存器数据信息
	// Parameter: long & nRegAddr : Output:寄存器地址
	// Parameter: int & nRegNum : Output:寄存器数量
	// Parameter: QByteArray & strCmdInfo : Output:指令信息(读、写)
	//************************************
	bool CheckCmdInfoValid(QByteArray& strInfo, long& nRegAddr, int& nRegNum, QByteArray& strCmdInfo);	//检查收到的信息是否符合MC协议二进制方式的3E帧数据格式，并截取寄存器数据部分返回

	virtual bool CmdInfoProcessing(const QByteArray& strInfo, ProcessType Curtype, QByteArray& strOut) override;



};

#endif //COMM_PROTOCOL_MITSUBISHIQBINARY_H


