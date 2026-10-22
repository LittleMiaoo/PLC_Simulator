/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef QUICKPANEL_H
#define QUICKPANEL_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QEvent>
#include <QVector>

class QuickPanel :
    public QDialog
{
    Q_OBJECT
public:
    explicit QuickPanel(QWidget* parent = nullptr);

	// 设置脚本按钮文本（从主窗口LineEdit获取,按现有按钮数量截断）
	void setButtonTexts(const QStringList& texts);

signals:
    // 通知主窗口显示的信号
    void showMainWindow();
    void executeLuaScript(int scriptIndex);   // 0-based

protected:
	// 重写关闭事件：直接关闭小窗时退出程序
	void closeEvent(QCloseEvent* event) override;
	void changeEvent(QEvent* event) override;

private:
    QVector<QPushButton*> btn;      // 脚本按钮
    QPushButton* btnExit = nullptr; // 退出小窗按钮
};

#endif // QUICKPANEL_H
