#include "CommBase.h"
#include <QMetaObject>

CommBase::CommBase(QObject* pParent)
	: QObject(pParent)
{
	m_threadPool = new QThreadPool(this);
	m_threadPool->setMaxThreadCount(QThread::idealThreadCount());
	m_requestTimeout = 1000;
}

void CommBase::AddToRequestQueue(const QString& endpointId,QByteArray&& data)
{
	{
		QMutexLocker locker(&m_queueMutex);
		PendingRequest request;
		request.endpointId = endpointId;
		request.requestData = std::move(data);
		request.timestamp = QDateTime::currentDateTime();
		request.isProcessing = false;
		request.requiresResponse = true;
		request.timeoutMs = m_requestTimeout;
		m_endpointQueues[endpointId].enqueue(request);
	}
	QTimer::singleShot(0, this, [=]{ ProcessNextForEndpoint(endpointId); });
}

void CommBase::ProcessNextForEndpoint(const QString& endpointId)
{
	{
		QMutexLocker locker(&m_queueMutex);
		if (m_endpointProcessing.contains(endpointId)) return;
		if (!m_endpointQueues.contains(endpointId) || m_endpointQueues[endpointId].isEmpty()) return;
		PendingRequest& head = m_endpointQueues[endpointId].head();
		head.isProcessing = true;
		m_endpointProcessing.insert(endpointId);
	}
	PendingRequest request;
	{
		QMutexLocker locker(&m_queueMutex);
		request = m_endpointQueues[endpointId].dequeue();
	}
	emit dataReceived(QString("Rece:[%1]:").arg(endpointId), request.requestData);

	class Task : public QRunnable {
	public:
		CommBase* self;
		QString id;
		QByteArray rec;
		Task(CommBase* s, const QString& i, const QByteArray& r) : self(s), id(i), rec(r) {}
		//move代替拷贝.提高性能
		Task(CommBase* s, QString&& i, QByteArray&& r) : self(s), id(std::move(i)), rec(std::move(r)) {}

		void run() override {
			if (!self->m_requestProcessor) {
				QMetaObject::invokeMethod(self, "OnTaskFinished", Qt::QueuedConnection, Q_ARG(QString, id));
				return;
			}
			QByteArray reply;
			bool ok = self->m_requestProcessor(rec, reply);
			if (ok) {
				self->SendDataToEndpoint(id, reply);
			}
			QMetaObject::invokeMethod(self, "OnTaskFinished", Qt::QueuedConnection, Q_ARG(QString, id));
		}
	};
	Task* t = new Task(this, std::move(request.endpointId), std::move(request.requestData));	//使用右值引用move版本, 提高性能
	t->setAutoDelete(true);
	m_threadPool->start(t);
}

void CommBase::OnTaskFinished(QString endpointId)
{
	{
		QMutexLocker locker(&m_queueMutex);
		m_endpointProcessing.remove(endpointId);
	}
	
	ProcessNextForEndpoint(endpointId);
}

