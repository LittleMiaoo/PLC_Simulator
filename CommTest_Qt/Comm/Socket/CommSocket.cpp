#include "CommSocket.h"
#include <QThread>
#include <QEventLoop>

CommSocket::CommSocket(QObject* pParent) : CommBase(pParent)
{
	m_bConnected = false;
	m_Server = nullptr;
	m_Client = nullptr;
	m_ClientMap.clear();
	m_requestTimeout = 1000;
	m_threadPool = new QThreadPool(this);
	m_threadPool->setMaxThreadCount(QThread::idealThreadCount());

	auto SendDataTask = [this](const QString& clientId, const QByteArray& data) {
		if (!m_bConnected) return false;
		QTcpSocket* targetClient = m_ClientMap.value(clientId, nullptr);
		if (!targetClient || targetClient->state() != QAbstractSocket::ConnectedState)
		{
			return false;
		}
		qint64 bytesWritten = targetClient->write(data);
		if (bytesWritten == -1)
		{
			return false;
		}

		emit dataSend(QString("Send:[%1]:").arg(clientId), data);
		return true;
	};
	connect(this, &CommSocket::dataSendRequest, this, SendDataTask, Qt::QueuedConnection);

}

CommSocket::~CommSocket()
{
	qDebug() << "CommSocket::~CommSocket()";
}

bool CommSocket::initializeServer()
{
	m_Server = new QTcpServer(this);

	//设置最大连接数
	int nListenNum = m_SocketInfo.m_nSocketListenNum > 0 ? m_SocketInfo.m_nSocketListenNum : 10;
	m_Server->setMaxPendingConnections(nListenNum);

	const QHostAddress addr = QHostAddress(m_SocketInfo.m_strSocketIPAddress);
	const uint16_t port = static_cast<uint16_t>(m_SocketInfo.m_nSocketPort);

	//服务器开始监听
	if (!m_Server->listen(addr, port))
	{
		return false;
	}

	//记录日志
	emit CommLogRecord(QString("[%1:%2] 服务器创建成功,开始监听客户端链接...")
		.arg(m_Server->serverAddress().toString())
		.arg(m_Server->serverPort()));


	m_bConnected = true;	//置位标志符

	//连接新的连接请求槽
	connect(m_Server, &QTcpServer::newConnection, this, [=] {
		//循环处理没有被处理的连接请求
		while (m_Server->hasPendingConnections())
		{
			QTcpSocket* Cursocket = m_Server->nextPendingConnection();
			QString clientId = GetClientId(Cursocket);

			m_ClientMap[clientId] = Cursocket;
			/*m_clientList.append(Cursocket);*/

			//激活信号,将新建立连接的客户端信息发送出去
			emit CommLogRecord(QString("[%1:%2] 新客户端链接")
				.arg(Cursocket->peerAddress().toString())
				.arg(Cursocket->peerPort()));

			//接收到数据时的操作
			connect(Cursocket, &QTcpSocket::readyRead, this, [=] {
				if (Cursocket->bytesAvailable() <= 0) return;

				QByteArray rec_Info = Cursocket->readAll();
				
				AddToRequestQueue(clientId, std::move(rec_Info));

				});

			//连接错误信息
			connect(Cursocket, &QAbstractSocket::errorOccurred, [this, Cursocket](QAbstractSocket::SocketError) {

				//发送错误信息
				emit CommLogRecord(QString("[%1:%2] 客户端错误:%3")
					.arg(Cursocket->peerAddress().toString())
					.arg(Cursocket->peerPort())
					.arg(Cursocket->errorString()));
				});

			//连接断开,销毁socket对象
			connect(Cursocket, &QTcpSocket::disconnected, [this, Cursocket] {

				Cursocket->deleteLater();

				//发送断开连接信息
				emit CommLogRecord(QString("[%1]:客户端断开链接").arg(GetClientId(Cursocket)));

				m_ClientMap.remove(GetClientId(Cursocket));


				});

		}
		});

	//server的错误信息
	connect(m_Server, &QTcpServer::acceptError, [this](QAbstractSocket::SocketError) {

		emit CommLogRecord(QString("服务器错误:%1").arg(m_Server->errorString()));
		});

	return true;
}

bool CommSocket::initializeClient()
{
	return false;
}

void CommSocket::Cleanup()
{

	emit CommLogRecord(QString("即将断开所有客户端链接,当前链接客户端数量[%1]").arg(m_ClientMap.size()));
	// 清理客户端连接
	for (QTcpSocket* client : m_ClientMap.values()) {
// 		client->close();
// 		client->deleteLater();
		emit CommLogRecord(QString("[%1:%2] 客户端即将断开").arg(client->peerAddress().toString())
			.arg(client->peerPort()));

		client->disconnectFromHost();
		if (client->state() != QAbstractSocket::UnconnectedState) client->abort();
	}

	if (m_Server) {
		emit CommLogRecord(QString("[%1:%2] 即将关闭服务器...")
			.arg(m_Server->serverAddress().toString())
			.arg(m_Server->serverPort()));

		m_Server->close();
		m_Server->deleteLater();
		m_Server = nullptr;
	}

	m_ClientMap.clear();

	// 清理客户端socket
	if (m_Client) {
		emit CommLogRecord(QString("[%1:%2] 即将关闭网络连接...").arg(GetClientId(m_Client)));

		m_Client->close();
		m_Client->deleteLater();
		m_Client = nullptr;
	}
}


bool CommSocket::Open(CommInfoBase* commInfo)
{
	//传参类型判断
	if (commInfo == nullptr || commInfo->GetCommType() != CommType::eSocket)
		return false;

	SocketCommInfo* socketInfo = static_cast<SocketCommInfo*>(commInfo);
	m_SocketInfo = *socketInfo;

	//若已有链接先关闭
	if (m_bConnected)
	{
		Close();
	}

	//判断当前网络通信的类型
	if (m_SocketInfo.m_SocketType == SocketType::eSTServer)
	{
		return initializeServer();
	}
	else
	{
		return initializeClient();
	}


	return false;
}

bool CommSocket::Close()
{
	{
		QMutexLocker locker(&m_queueMutex);
		m_endpointQueues.clear();
		m_endpointProcessing.clear();
	}
	Cleanup();
	return true;
}

bool CommSocket::SendData(const QByteArray& strData)
{
	return false;
}

 

bool CommSocket::SendDataToEndpoint(const QString& clientId, const QByteArray& strData)
{
	if (!m_bConnected) return false;
//     QTcpSocket* targetClient = m_ClientMap.value(clientId, nullptr);
//     if (!targetClient || targetClient->state() != QAbstractSocket::ConnectedState)
//     {
//         return false;
//     }
//     qint64 bytesWritten = targetClient->write(strData);
//     if (bytesWritten == -1)
//     {
//         return false;
//     }
// 	if (!targetClient->flush()) {
// 		qDebug() << "Flush failed for client:" << clientId;
// 	}
// 	qDebug() << "Sent" << bytesWritten << "bytes to client:" << clientId
// 		<< "Data:" << strData; 
//	emit dataSend(QString("Send:[%1]:").arg(clientId), strData);

	emit dataSendRequest(clientId, strData);
    return true;
}


// CommBase::CommStatus CommSocket::RecieveData(QString& strData)
// {
// 	return CommStatus();
// }

