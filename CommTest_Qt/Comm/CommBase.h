#ifndef COMM_BASE_H
#define COMM_BASE_H
#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadPool>
#include <QSet>
#include <QDateTime>
#include <QTimer>
#include <QMap>
#include <QByteArray>
#include <QString>
#include <functional>
#include <QRunnable>
#include <QThread>
class CommBase : public QObject
{
	Q_OBJECT

signals:
	void CommLogRecord(const QString& data);	//日志信号
	void dataReceived(const QString& objectInfo,const QByteArray& data); //接收到数据的信号
	void dataSend(const QString& objectInfo, const QByteArray& data);	 //发送数据信号

	void dataSendRequest(const QString& endpointId, const QByteArray& data);	//发送请求信号
public:

	//20251030	wm	通信类型
	enum class CommType
	{
		eCommUnknown = -1,	// 未知类型	
		eSerial,			// 串口
		eSocket,			// 网络
	};

	//20251030	wm	通信状态
	enum class CommStatus
	{
		eCommSucceed = 0,   // 成功
		eCommErr = 1,       // 通信错误
		eCommUnConnect,		// 通信未连接
		eCommStop,			// 通信停止
		eCommTimeOut,       // 通信超时
	};

	struct CommInfoBase
	{
		QString		m_strCommStop;			//通信终止符
		QString		m_strCmdStop;			//命令终止符

		CommInfoBase()
		{
			m_strCommStop = "";
			m_strCmdStop = "";
		}

		virtual CommType GetCommType() = 0;	  // 获取通讯类型
		virtual ~CommInfoBase() = default;
	};

public:
	explicit CommBase(QObject* pParent = nullptr);
	virtual ~CommBase() = default;
	CommBase(const CommBase&) = delete;

	virtual bool Open(CommInfoBase* commInfo) = 0;								// 打开连接
	virtual bool Close() = 0;													// 关闭连接
	virtual bool IsOpen() = 0;													// 连接是否打开
	virtual bool SendData(const QByteArray& strData ) = 0;							// 发送数据
	//virtual CommStatus RecieveData(QString& strData) = 0;						// 接收数据
public:
	void SetRequestProcessor(std::function<bool(const QByteArray&, QByteArray&)> fn) { m_requestProcessor = std::move(fn); }
protected:
	struct PendingRequest {
		QString endpointId;
		QByteArray requestData;
		QDateTime timestamp;
		bool isProcessing;
		bool requiresResponse;
		int timeoutMs;
	};
	void AddToRequestQueue(const QString& endpointId,/* const QByteArray& data*/QByteArray&& data);	// 添加请求.传参改为右值引用, 避免拷贝
	void ProcessNextForEndpoint(const QString& endpointId);
	virtual bool SendDataToEndpoint(const QString& endpointId, const QByteArray& data) = 0;
	Q_INVOKABLE void OnTaskFinished(QString endpointId);
protected:
	QMap<QString, QQueue<PendingRequest>> m_endpointQueues; //每个客户端对应一个队列, 队列中存放的是待处理的请求
	QSet<QString> m_endpointProcessing;	//正在处理的客户端
	QMutex m_queueMutex;
	QThreadPool* m_threadPool;
	int m_requestTimeout;
	std::function<bool(const QByteArray&, QByteArray&)> m_requestProcessor;
};




#endif //COMM_BASE_H


