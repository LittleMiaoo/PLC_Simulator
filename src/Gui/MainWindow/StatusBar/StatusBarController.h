/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef STATUSBARCONTROLLER_H
#define STATUSBARCONTROLLER_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QPointer>
#include <functional>

#include "Comm/CommEvent.h"          // CommEvent / CommDirection
#include "Core/PlatformTypes.h"      // Platform / Pose
#include "AuxDialogs.h"              // AuxDialogs::PlatformParams

class QStatusBar;
class StatusSegment;

// 状态栏控制器:持有 5 段 StatusSegment,订阅各源信号、聚合计数、格式化并刷新。
// 拥有全部格式化逻辑;MainWindow 只负责把信号连到本类槽,并注入观察不到的配置
// (通信配置 setCommConfig、平台参数 setPlatformParams)。
class StatusBarController : public QObject
{
    Q_OBJECT

public:
    // bar: 主界面状态栏;parent: 父对象
    explicit StatusBarController(QStatusBar* bar, QObject* parent = nullptr);

    // 各源信号 → 槽(均在 GUI 线程执行;通信/位姿为直连,仅 scriptFinished 来自线程池为 Queued)
    void onConnectionStateChanged(bool listening);
    void onClientsChanged(const QStringList& clientIds);
    void onCommEvent(const CommEvent& ev);
    void onCommTimeout(const QString& endpointId);
    void onScriptStarted(int index);
    void onScriptFinished(int index, bool ok, const QString& err);
    void onPoseChanged(Platform which, const Pose& pose);

    // 注入控制器观察不到的配置
    void setCommConfig(const QString& protocol, const QString& ip, quint16 port, bool isServer);
    void setPlatformParams(const AuxDialogs::PlatformParams& params);
    // 按脚本 index 取当前名称(读名称框);空则脚本段悬浮回退 "脚本 #N"
    void setScriptNameProvider(std::function<QString(int)> provider);

    void refreshAll();

private:
    // 刷新可见值(热路径,仅算紧凑串 + setValue)
    void refreshConnection();
    void refreshClients();
    void refreshHealth();
    void refreshScript();
    void refreshPlatform();

    // 详情惰性构建(仅悬浮时调用,读实时状态;返回空则不显示 tooltip)
    QString buildConnDetail() const;
    QString buildClientsDetail() const;
    QString buildHealthDetail() const;
    QString buildScriptDetail() const;
    QString buildPlatformDetail() const;

    static QString formatBytes(quint64 n);

    // 5 段(父=状态栏)
    StatusSegment* m_segConn    = nullptr;
    StatusSegment* m_segClients = nullptr;
    StatusSegment* m_segHealth  = nullptr;
    StatusSegment* m_segScript  = nullptr;
    StatusSegment* m_segPlatform = nullptr;

    // 通信连接
    bool        m_listening = false;
    bool        m_isServer  = true;
    QString     m_protocol;
    QString     m_ip;
    quint16     m_port = 0;
    QStringList m_clients;

    // 通信健康(累计;通信重开时清零)
    quint64 m_rxFrames = 0, m_txFrames = 0;
    quint64 m_rxBytes  = 0, m_txBytes  = 0;
    int     m_timeouts = 0;
    QString m_lastActivity;

    // 脚本(index → 运行中次数;引用计数容忍同 index 并发 + 名称取值器)
    QHash<int, int> m_running;
    std::function<QString(int)> m_scriptNameProvider;

    // 平台
    Pose m_live;
    Pose m_base;
    AuxDialogs::PlatformParams m_params{3, 3, 114, 120};
};

#endif // STATUSBARCONTROLLER_H
