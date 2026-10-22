/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef COLLAPSIBLEGROUPBOX_H
#define COLLAPSIBLEGROUPBOX_H

#include <QGroupBox>
#include <QPointer>

class QToolButton;

/**
 * @brief 可折叠分组框
 *
 * 继承 QGroupBox 以便在 Qt Designer 中由普通 QGroupBox 提升(promote)而来。
 * 点击标题栏切换折叠:折叠时隐藏所有子控件并将高度收缩至标题行,
 * 由父布局自动重排(下方控件上移、带伸缩因子的控件撑大)。
 * 标题前以 ▾/▸ 指示展开/折叠状态。
 *
 * 可选关闭按钮:调用 setClosable(true) 后,标题栏右侧出现 × 按钮,
 * 点击时发出 closed() 信号(不自动隐藏控件,由调用方决定行为)。
 * 默认不启用,主窗口现有用法不受影响。
 */
class CollapsibleGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit CollapsibleGroupBox(QWidget* parent = nullptr);

    void setCollapsed(bool collapsed);
    bool isCollapsed() const { return m_collapsed; }

    void setClosable(bool closable);          // 开启后标题栏右侧显示 × 关闭按钮
    bool isClosable() const { return m_closable; }

    // 隐藏 QGroupBox::setTitle(非虚):记录原始标题并保留 ▾/▸ 前缀。
    // uic 的 retranslateUi 以本类指针调用,切换语言后箭头不丢、折叠时也不会退回旧标题;
    // 经 QGroupBox* 调用会绕过本函数,勿那样设置标题
    void setTitle(const QString& title);

signals:
    void closed();                            // 点击 × 时发出

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* e) override;
    void changeEvent(QEvent* event) override;

private:
    void refreshTitlePrefix();
    int  headerHeight() const;
    void positionCloseButton();

    QString m_baseTitle;
    bool    m_collapsed = false;
    bool    m_titleCaptured = false;
    bool    m_closable = false;
    QToolButton* m_closeButton = nullptr;

    QList<QPointer<QWidget>> m_hiddenOnCollapse;  // 折叠时被本控件隐藏的子控件,展开时仅恢复这些
};

#endif // COLLAPSIBLEGROUPBOX_H
