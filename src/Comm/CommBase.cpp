/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "CommBase.h"
#include <QCoreApplication>
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
		if (m_endpointQueues[endpointId].size() >= m_maxQueueSizePerEndpoint) {
			emit logRecord(QCoreApplication::translate("CommBase", "队列溢出，丢弃请求: %1").arg(endpointId));
			return;
		}
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
	// 超时检查
	if (request.timestamp.msecsTo(QDateTime::currentDateTime()) > request.timeoutMs) {
		emit logRecord(QCoreApplication::translate("CommBase", "请求超时，跳过: %1").arg(endpointId));
		emit commTimeout(endpointId);	//状态栏:超时计数
		QMetaObject::invokeMethod(this, "OnTaskFinished", Qt::QueuedConnection, Q_ARG(QString, endpointId));
		return;
	}
	CommEvent ev;
	ev.direction  = CommDirection::eReceive;
	ev.endpointId = endpointId;
	ev.bytes      = request.requestData;   // 复制:request 随后仍被移动入 CommTask
	ev.timestamp  = QDateTime::currentDateTime();
	emit commEvent(ev);

	class CommTask : public QRunnable {
	public:
		CommBase* self;
		QString id;
		QByteArray rec;
		CommTask(CommBase* s, const QString& i, const QByteArray& r) : self(s), id(i), rec(r) {}
		//提供移动语义的构造函数
		CommTask(CommBase* s, QString&& i, QByteArray&& r) : self(s), id(std::move(i)), rec(std::move(r)) {}

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
	CommTask* t = new CommTask(this, std::move(request.endpointId), std::move(request.requestData));	//使用移动语义的构造函数，避免拷贝
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

