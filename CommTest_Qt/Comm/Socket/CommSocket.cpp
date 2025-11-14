#include "CommSocket.h"

CommSocket::CommSocket(QObject* pParent) : CommBase(pParent)
{
	m_bConnected = false;
	m_Server = nullptr;
	m_Client = nullptr;
	m_ClientMap.clear();
	m_processingRequest = false;

	m_requestTimeout = 1000;
	m_timeoutTimer = new QTimer(this);
	m_timeoutTimer->setSingleShot(true);
	connect(m_timeoutTimer, &QTimer::timeout, this, [=] {

		QMutexLocker locker(&m_queueMutex);

		if (m_processingRequest && !m_requestQueue.isEmpty()) {
			PendingRequest& currentRequest = m_requestQueue.head();
			if (currentRequest.isProcessing) {
				emit CommLogRecord(QString("[%1]:请求超时").arg(currentRequest.clientId));

				// 移除超时的请求
				m_requestQueue.dequeue();
				m_processingRequest = false;
				m_currentClientId.clear();
			}
		}

		locker.unlock();

		// 处理下一个请求
		ProcessNextRequest();

		});

}

CommSocket::~CommSocket()
{
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
				
				AddToRequestQueue(clientId, rec_Info);

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
	// 清空请求队列
	{
		QMutexLocker locker(&m_queueMutex);
		m_requestQueue.clear();
		m_processingRequest = false;
		m_currentClientId.clear();
	}

	Cleanup();

	return true;
}

bool CommSocket::SendData(const QByteArray& strData)
{
	if (!m_bConnected) return false;
	
	QString strCurClientId;

	//缩小互斥锁的作用范围
	{
		QMutexLocker locker(&m_queueMutex);

		// 如果没有正在处理的请求，直接返回失败
		if (m_requestQueue.isEmpty() || !m_processingRequest) 
		{
			return false;
		}

		// 获取当前正在处理的请求
		PendingRequest& currentRequest = m_requestQueue.head();
		if (!currentRequest.isProcessing) 
		{
			return false;
		}
		strCurClientId = currentRequest.clientId;

		// 标记请求处理完成
		m_requestQueue.dequeue();
		m_processingRequest = false;
		m_currentClientId.clear();

	}
	
	// 停止超时定时器
	m_timeoutTimer->stop();


	// 查找对应的客户端
	QTcpSocket* targetClient = m_ClientMap.value(strCurClientId, nullptr);

	if (!targetClient || targetClient->state() != QAbstractSocket::ConnectedState)
	{
		QTimer::singleShot(0, this, &CommSocket::ProcessNextRequest);
		return false;
	}

	// 发送数据
	qint64 bytesWritten = targetClient->write(strData);
	if (bytesWritten == -1) 
	{
		QTimer::singleShot(0, this, &CommSocket::ProcessNextRequest);
		return false;
	}

	emit dataSend(QString("Send:[%1]:").arg(strCurClientId), strData);



	// 处理下一个请求
	QTimer::singleShot(0, this, &CommSocket::ProcessNextRequest);

	return true;
}

void CommSocket::AddToRequestQueue(const QString& clientId, const QByteArray& data)
{
	//缩小互斥锁的作用范围
	{
		QMutexLocker locker(&m_queueMutex);

		PendingRequest request;
		request.clientId = clientId;
		request.requestData = data;

		request.timestamp = QDateTime::currentDateTime();
		request.isProcessing = false;
		request.requiresResponse = true; // 默认需要回复
		request.timeoutMs = m_requestTimeout;

		m_requestQueue.enqueue(request);
	}
	
	// 如果没有正在处理的请求，立即处理
	if (!m_processingRequest) {
		QTimer::singleShot(0, this, &CommSocket::ProcessNextRequest);
	}
}

void CommSocket::ProcessNextRequest()
{
	// 如果已经在处理请求或者队列为空，直接返回
	if (m_processingRequest || m_requestQueue.isEmpty()) {
		return;
	}

	QString strobjectInfo;
	QByteArray strDataRec;
	bool requiresResponse = true;

	//缩小互斥锁的作用范围
	{
		QMutexLocker locker(&m_queueMutex);
		// 取出下一个请求
		PendingRequest& request = m_requestQueue.head();
		request.isProcessing = true;
		m_processingRequest = true;
		m_currentClientId = request.clientId;
		requiresResponse = request.requiresResponse;

		strDataRec = request.requestData;
		
		strobjectInfo = QString("Rece:[%1]:").arg(request.clientId);
	}

	if (requiresResponse) 
	{
		m_timeoutTimer->start(m_requestTimeout);
	}

	emit dataReceived(strobjectInfo,strDataRec);
}


// CommBase::CommStatus CommSocket::RecieveData(QString& strData)
// {
// 	return CommStatus();
// }

