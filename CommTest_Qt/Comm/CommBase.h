#ifndef COMM_BASE_H
#define COMM_BASE_H
#include <QObject>
class CommBase : public QObject
{
	Q_OBJECT

signals:
	void CommLogRecord(const QString& data);	//日志信号
	void dataReceived(const QString& objectInfo,const QByteArray& data); //接收到数据的信号
	void dataSend(const QString& objectInfo, const QByteArray& data);	 //发送数据信号

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
};




#endif //COMM_BASE_H


