#include "CommBase.h"
#include <QMetaObject>

CommBase::CommBase(QObject* pParent)
	: QObject(pParent)
{
	m_threadPool = new QThreadPool(this);
	m_threadPool->setMaxThreadCount(QThread::idealThreadCount());
	m_requestTimeout = 1000;
}

void CommBase::AddToRequestQueue(const QString& endpointId, const QByteArray& data)
{
	{
		QMutexLocker locker(&m_queueMutex);
		PendingRequest request;
		request.endpointId = endpointId;
		request.requestData = data;
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
	Task* t = new Task(this, endpointId, request.requestData);
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

