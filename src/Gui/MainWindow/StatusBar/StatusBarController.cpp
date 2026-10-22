/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "StatusBarController.h"
#include "StatusSegment.h"
#include "ThemeManager.h"

#include <QStatusBar>
#include <QList>
#include <QColor>
#include <algorithm>

namespace {
// 语义状态色:跨主题恒定(绿=连接、蓝=TX、绿=RX)
constexpr auto kGreen = "#22c55e";
constexpr auto kTx    = "#2563eb";
constexpr auto kRx    = "#16a34a";

// 中性色跟随主题:运行时取当前主题 token 的实际色值(@text 值/标题,@text2 键/灰显)
QString themed(const char* token) { return ThemeManager::instance().color(token).name(); }

// 悬停详情:两列对齐表(键次要色左、值主色右)
QString kvRow(const QString& key, const QString& value)
{
    return QString("<tr><td style='color:%1;padding-right:16px'>%2</td>"
                   "<td style='color:%3'>%4</td></tr>").arg(themed("@text2"), key, themed("@text"), value);
}
QString kvTable(const QString& rows)
{
    return "<table cellspacing='3'>" + rows + "</table>";
}
// 列表类详情:加粗标题 + 每行一项
QString titledList(const QString& title, const QStringList& items)
{
    QString s = QString("<b style='color:%1'>%2</b>").arg(themed("@text"), title);
    for (const QString& it : items)
        s += QString("<div style='color:%1;padding-top:2px'>%2</div>").arg(themed("@text"), it);
    return s;
}
}

StatusBarController::StatusBarController(QStatusBar* bar, QObject* parent)
    : QObject(parent)
{
    m_segConn     = new StatusSegment(bar);
    m_segClients  = new StatusSegment(bar);
    m_segHealth   = new StatusSegment(bar);
    m_segScript   = new StatusSegment(bar);
    m_segPlatform = new StatusSegment(bar);
    
    // 段 1~4 靠左(消息区侧),平台段靠右(常驻区)
    bar->addWidget(m_segConn);
    bar->addWidget(m_segClients);
    bar->addWidget(m_segHealth);
    bar->addWidget(m_segScript);
    bar->addPermanentWidget(m_segPlatform);

    // 详情惰性构建:各段装一次 provider,悬浮时才读实时状态拼 HTML。
    // QPointer 守卫控制器已析构但段仍在的极端窗口(实测销毁期无悬浮,纯防御)。
    m_segConn->setDetailProvider    ([self = QPointer<StatusBarController>(this)]{ return self ? self->buildConnDetail()     : QString(); });
    m_segClients->setDetailProvider ([self = QPointer<StatusBarController>(this)]{ return self ? self->buildClientsDetail()  : QString(); });
    m_segHealth->setDetailProvider  ([self = QPointer<StatusBarController>(this)]{ return self ? self->buildHealthDetail()   : QString(); });
    m_segScript->setDetailProvider  ([self = QPointer<StatusBarController>(this)]{ return self ? self->buildScriptDetail()   : QString(); });
    m_segPlatform->setDetailProvider([self = QPointer<StatusBarController>(this)]{ return self ? self->buildPlatformDetail() : QString(); });

    refreshAll();

    // 主题切换:重刷各段可见值,使内联中性色按新主题 token 取色(语义状态色不变;
    // tooltip 详情因悬浮时才取色,天然已随主题)。
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
        refreshAll();
    });
}

void StatusBarController::refreshAll()
{
    refreshConnection();
    refreshClients();
    refreshHealth();
    refreshScript();
    refreshPlatform();
}

// ---- 槽:各源信号 ----

void StatusBarController::onConnectionStateChanged(bool listening)
{
    m_listening = listening;
    if (listening)
    {
        // 通信重开:健康计数清零(每次会话独立)
        m_rxFrames = m_txFrames = 0;
        m_rxBytes = m_txBytes = 0;
        m_timeouts = 0;
        m_lastActivity.clear();
    }
    else
    {
        m_clients.clear();
    }
    // 三段随连接状态统一重画(断开时健康段才会走灰显分支)
    refreshConnection();
    refreshClients();
    refreshHealth();
}

void StatusBarController::onClientsChanged(const QStringList& clientIds)
{
    m_clients = clientIds;
    refreshClients();
}

void StatusBarController::onCommEvent(const CommEvent& ev)
{
    if (ev.direction == CommDirection::eReceive)
    {
        ++m_rxFrames;
        m_rxBytes += static_cast<quint64>(ev.bytes.size());
    }
    else
    {
        ++m_txFrames;
        m_txBytes += static_cast<quint64>(ev.bytes.size());
    }
    if (ev.timestamp.isValid())
        m_lastActivity = ev.timestamp.toString("hh:mm:ss");
    refreshHealth();
}

void StatusBarController::onCommTimeout(const QString& /*endpointId*/)
{
    ++m_timeouts;
    refreshHealth();
}

void StatusBarController::onScriptStarted(int index)
{
    ++m_running[index];   // 同 index 并发时累加运行次数
    refreshScript();
}

void StatusBarController::onScriptFinished(int index, bool /*ok*/, const QString& /*err*/)
{
    auto it = m_running.find(index);
    if (it != m_running.end() && --it.value() <= 0)   // 减到 0 才移出,容忍成对乱序保护
        m_running.erase(it);
    refreshScript();
}

void StatusBarController::onPoseChanged(Platform which, const Pose& pose)
{
    if (which == Platform::Live) m_live = pose;
    else                         m_base = pose;
    refreshPlatform();
}

// ---- 配置注入 ----

void StatusBarController::setCommConfig(const QString& protocol, const QString& ip, quint16 port, bool isServer)
{
    m_protocol = protocol;
    m_ip       = ip;
    m_port     = port;
    m_isServer = isServer;
    refreshConnection();
    refreshClients();
}

void StatusBarController::setPlatformParams(const AuxDialogs::PlatformParams& params)
{
    m_params = params;
    refreshPlatform();
}

void StatusBarController::setScriptNameProvider(std::function<QString(int)> provider)
{
    m_scriptNameProvider = std::move(provider);
    refreshScript();
}

// ---- 刷新(格式化)----

void StatusBarController::refreshConnection()
{
    if (m_listening)
    {
        const QString dot  = QString("<span style='color:%1'>●</span>").arg(kGreen);
        const QString role = m_isServer ? tr("服务器") : tr("客户端");
        m_segConn->setValue(QString("%1 %2 %3:%4").arg(dot, role, m_ip).arg(m_port));
    }
    else
    {
        const QString grey = themed("@text2");
        const QString dot  = QString("<span style='color:%1'>○</span>").arg(grey);
        m_segConn->setValue(QString("%1 <span style='color:%2'>%3</span>").arg(dot, grey, tr("未连接")));
    }
}

void StatusBarController::refreshClients()
{
    if (!m_listening || !m_isServer)
    {
        m_segClients->setValue(tr("客户端: <span style='color:%1'>—</span>").arg(themed("@text2")));
        return;
    }
    m_segClients->setValue(tr("客户端: %1").arg(m_clients.size()));
}

void StatusBarController::refreshHealth()
{
    QString value;
    if (m_listening)
        // 标签序 RX TX 与其后"收/发"数字一一对齐(RX=收=绿, TX=发=蓝)
        value = QString("<span style='font-size:16px'>⇅</span> <span style='color:%1'>RX</span> <span style='color:%2'>TX</span> %3%4/%5%6")
                    .arg(kRx)
                    .arg(kTx)
                    .arg(tr("收"))
                    .arg(m_rxFrames)
                    .arg(tr("发"))
                    .arg(m_txFrames);
    else
        value = QString("<span style='color:%1'><span style='font-size:16px'>⇅</span> RX TX %2%3/%4%5</span>")
                    .arg(themed("@text2"))
                    .arg(tr("收"))
                    .arg(m_rxFrames)
                    .arg(tr("发"))
                    .arg(m_txFrames);
    m_segHealth->setValue(value);
}

void StatusBarController::refreshScript()
{
    if (m_running.isEmpty())
    {
        m_segScript->setValue(tr("脚本: <span style='color:%1'>空闲</span>").arg(themed("@text2")));
        return;
    }
    m_segScript->setValue(tr("脚本: %1 运行中").arg(m_running.size()));
}

void StatusBarController::refreshPlatform()
{
    m_segPlatform->setValue(
        tr("平台 X:%1 Y:%2 θ:%3")
            .arg(m_live.x, 0, 'f', 2).arg(m_live.y, 0, 'f', 2).arg(m_live.angleDeg, 0, 'f', 1));
}

// ---- 详情惰性构建(悬浮时调用,读实时状态)----

QString StatusBarController::buildConnDetail() const
{
    if (!m_listening && m_protocol.isEmpty())
        return QString();   // 未连接且未配过通信:无详情
    // 协议/角色/地址/端口 两态共用;仅角色文案(监听中)与末尾"状态"行有差异
    const QString roleDetail = m_isServer
        ? (m_listening ? tr("服务器(监听中)") : tr("服务器"))
        : tr("客户端");
    QString rows = kvRow(tr("协议"), m_protocol)
                 + kvRow(tr("角色"), roleDetail)
                 + kvRow(tr("地址"), m_ip)
                 + kvRow(tr("端口"), QString::number(m_port));
    if (!m_listening)
        rows += kvRow(tr("状态"), tr("未监听"));
    return kvTable(rows);
}

QString StatusBarController::buildClientsDetail() const
{
    if (!m_listening || !m_isServer || m_clients.isEmpty())
        return QString();
    return titledList(tr("已连接客户端 (%1)").arg(m_clients.size()), m_clients);
}

QString StatusBarController::buildHealthDetail() const
{
    QString rows = kvRow(tr("收 / 发 帧"), QString("%1 / %2").arg(m_rxFrames).arg(m_txFrames))
                 + kvRow(tr("收 / 发 字节"), QString("%1 / %2").arg(formatBytes(m_rxBytes), formatBytes(m_txBytes)))
                 + kvRow(tr("超时"), QString::number(m_timeouts));
    if (!m_lastActivity.isEmpty())
        rows += kvRow(tr("最近"), m_lastActivity);
    return kvTable(rows);
}

QString StatusBarController::buildScriptDetail() const
{
    if (m_running.isEmpty())
        return QString();
    QList<int> idxs = m_running.keys();   // 运行中脚本 index(值为次数,此处只取键)
    std::sort(idxs.begin(), idxs.end());
    QStringList items;
    for (int i : idxs)
    {
        const QString name = m_scriptNameProvider ? m_scriptNameProvider(i).trimmed() : QString();
        items << (name.isEmpty() ? tr("脚本 #%1").arg(i + 1)   // 空名回退到 LuaFile{index+1}
                                 : name.toHtmlEscaped());           // 用户文本,转义防破坏 HTML 悬浮
    }
    return titledList(tr("运行中脚本 (%1)").arg(idxs.size()), items);
}

QString StatusBarController::buildPlatformDetail() const
{
    auto poseStr = [](const Pose& p) {
        return QString("X %1  Y %2  θ %3°")
            .arg(p.x, 0, 'f', 2).arg(p.y, 0, 'f', 2).arg(p.angleDeg, 0, 'f', 1);
    };
    return kvTable(kvRow(tr("Live 位姿"), poseStr(m_live))
                 + kvRow(tr("Base 位姿"), poseStr(m_base))
                 + kvRow(tr("单位幂 XY / D"), QString("10^%1 / 10^%2").arg(m_params.unitXY).arg(m_params.unitD))
                 + kvRow(tr("对象轴写入"), QString("D%1").arg(m_params.objAddr))
                 + kvRow(tr("目标轴写入"), QString("D%1").arg(m_params.tgtAddr)));
}

QString StatusBarController::formatBytes(quint64 n)
{
    if (n < 1024)
        return QString("%1 B").arg(n);
    const double kb = n / 1024.0;
    if (kb < 1024.0)
        return QString("%1 KB").arg(kb, 0, 'f', 1);
    return QString("%1 MB").arg(kb / 1024.0, 0, 'f', 1);
}
