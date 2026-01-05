/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao <mao.wang.dev@foxmail.com>
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef COMM_SOCKET_H
#define COMM_SOCKET_H
#include "CommBase.h"

#include <QTcpSocket>
#include <QTcpServer>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QDateTime>
#include <QUuid>
#include <QThreadPool>
#include <QSet>
#include <functional>

class CommSocket : public CommBase
{
	Q_OBJECT

public:
	//网络通信类型
	enum class SocketType
	{
		eSTUnknown = -1,	// 未知类型	
		eSTServer,			// 服务端
		eSTClient			// 客户端
	};

	//网络通信需要的关键参数
	struct SocketCommInfo :public CommInfoBase
	{
		SocketType	m_SocketType;			//网络类型
		QString		m_strSocketIPAddress;	//IP地址
		uint32_t	m_nSocketPort;			//端口号
		uint32_t	m_nSocketListenNum;		//监听数
		

		SocketCommInfo()
		{
			m_SocketType = SocketType::eSTServer;
			m_strSocketIPAddress = "0.0.0.0";
			m_nSocketPort = 2000;
			m_nSocketListenNum = 10;
			
		};

// 		SocketCommInfo(const SocketCommInfo&) = default;
// 		SocketCommInfo& operator=(const SocketCommInfo&) = default;

		virtual CommType GetCommType() override { return CommType::eSocket; }
		virtual ~SocketCommInfo() {}
	};

public:
    explicit CommSocket(QObject* pParent);
	CommSocket(const CommSocket&) = delete;
	CommSocket& operator=(const CommSocket&) = delete;
	virtual ~CommSocket();

	//继承的基类的纯虚接口实现
public:
	virtual bool Open(CommInfoBase* commInfo) override;								// 打开连接
	virtual bool Close() override;													// 关闭连接
	virtual bool SendData(const QByteArray& strData) override;							// 发送数据
	//virtual CommStatus RecieveData(QString& strData);						// 接收数据
    virtual bool IsOpen() override { return m_bConnected; }

private:
	bool initializeServer();	//服务器初始化
	bool initializeClient();	//客户端初始化

private:
	void Cleanup();
	
    void ProcessNextRequest();

	QString GetClientId(QTcpSocket* client) const
	{
		if (!client) return QString();
		return QString("%1:%2").arg(client->peerAddress().toString())
			.arg(client->peerPort());
	}


    SocketCommInfo m_SocketInfo;		//网络通信的信息
    QTcpServer* m_Server;				//网络通信类型为服务器时使用
    QTcpSocket* m_Client;				//网络通信类型为客户端时使用
    QMap<QString, QTcpSocket*> m_ClientMap;	//连接到服务器的客户端

	bool m_bConnected;	//当前是否已有链接

    bool SendDataToEndpoint(const QString& clientId, const QByteArray& strData) override;
};

#endif	//COMM_SOCKET_H



