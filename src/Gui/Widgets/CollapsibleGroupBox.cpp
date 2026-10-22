/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "CollapsibleGroupBox.h"

#include <QMouseEvent>
#include <QEvent>
#include <QFontMetrics>
#include <QToolButton>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionGroupBox>

CollapsibleGroupBox::CollapsibleGroupBox(QWidget* parent)
    : QGroupBox(parent)
{
    setCheckable(false);  // 防御:不使用原生 checkable 指示器
    // 延迟到首次显示时记录原始标题并加箭头前缀
}

bool CollapsibleGroupBox::event(QEvent* e)
{
    // 在 polish(样式/标题就绪)后初始化标题前缀
    if (e->type() == QEvent::Polish && !m_titleCaptured)
    {
        m_baseTitle = title();
        m_titleCaptured = true;
        refreshTitlePrefix();
    }
    return QGroupBox::event(e);
}

int CollapsibleGroupBox::headerHeight() const
{
    // 标题行高度:字体高度 + 上下内边距余量
    return fontMetrics().height() + 18;
}

void CollapsibleGroupBox::refreshTitlePrefix()
{
    const QString arrow = m_collapsed ? QStringLiteral("▸ ")   // ▸
                                      : QStringLiteral("▾ ");   // ▾
    QGroupBox::setTitle(arrow + m_baseTitle);
}

void CollapsibleGroupBox::setTitle(const QString& title)
{
    m_baseTitle = title;
    m_titleCaptured = true;   // 已显式记录,Polish 时不再从 title() 回捕(否则会把箭头前缀当成原标题)
    refreshTitlePrefix();
}

void CollapsibleGroupBox::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange && m_closeButton)
        m_closeButton->setToolTip(tr("关闭"));
    QGroupBox::changeEvent(event);
}

void CollapsibleGroupBox::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed)
    {
        return;
    }
    m_collapsed = collapsed;

    if (collapsed)
    {
        m_hiddenOnCollapse.clear();
        const QList<QWidget*> kids = findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
        for (QWidget* w : kids)
        {
            if (w == m_closeButton)   // 关闭按钮常驻标题栏,折叠时不隐藏
                continue;
            if (w->isVisible())
            {
                w->hide();
                m_hiddenOnCollapse.append(w);
            }
        }
        setMaximumHeight(headerHeight());
    }
    else
    {
        for (const QPointer<QWidget>& w : m_hiddenOnCollapse)
        {
            if (w)
            {
                w->show();
            }
        }
        m_hiddenOnCollapse.clear();
        setMaximumHeight(QWIDGETSIZE_MAX);
    }
    refreshTitlePrefix();
}

void CollapsibleGroupBox::mousePressEvent(QMouseEvent* event)
{
    // 仅当点击落在标题行区域时切换折叠
    if (event->button() == Qt::LeftButton && event->position().toPoint().y() <= headerHeight())
    {
        setCollapsed(!m_collapsed);
        event->accept();
        return;
    }
    QGroupBox::mousePressEvent(event);
}

void CollapsibleGroupBox::setClosable(bool closable)
{
    if (m_closable == closable)
        return;
    m_closable = closable;

    if (closable && !m_closeButton)
    {
        m_closeButton = new QToolButton(this);
        m_closeButton->setObjectName(QStringLiteral("groupCloseButton"));
        m_closeButton->setText(QStringLiteral("×"));   // ×
        m_closeButton->setToolTip(tr("关闭"));
        m_closeButton->setCursor(Qt::PointingHandCursor);
        m_closeButton->setAutoRaise(true);
        m_closeButton->setFocusPolicy(Qt::NoFocus);
        m_closeButton->setFixedSize(18, 18);
        connect(m_closeButton, &QToolButton::clicked, this, &CollapsibleGroupBox::closed);
        positionCloseButton();
        m_closeButton->show();
    }
    else if (m_closeButton)
    {
        m_closeButton->setVisible(closable);
    }
}

void CollapsibleGroupBox::positionCloseButton()
{
    if (!m_closeButton)
        return;
    // 用 Qt 样式真实的标题矩形对齐,使 × 与标题文字垂直同高
    QStyleOptionGroupBox opt;
    initStyleOption(&opt);
    const QRect titleRect = style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, this);
    const int margin = 6;
    const int x = width() - m_closeButton->width() - margin;
    const int y = titleRect.center().y() - m_closeButton->height() / 2;
    m_closeButton->move(qMax(0, x), qMax(0, y));
}

void CollapsibleGroupBox::resizeEvent(QResizeEvent* event)
{
    QGroupBox::resizeEvent(event);
    positionCloseButton();
}
