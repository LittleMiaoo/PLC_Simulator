/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao <mao.wang.dev@foxmail.com>
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QCloseEvent>

class SubMainWindow :
    public QDialog
{
    Q_OBJECT
public:
    explicit SubMainWindow(QWidget* parent = nullptr);

	// 设置6个按钮的文本（从主窗口LineEdit获取）
	void setButtonTexts(const QStringList& texts);

signals:
    // 通知主窗口显示的信号
    void showMainWindow();
    void executeLuaScript(int buttonId);

protected:
	// 重写关闭事件：直接关闭小窗时退出程序
	void closeEvent(QCloseEvent* event) override {
		Q_UNUSED(event);
		qApp->quit(); // 退出整个应用程序
	}

private:
    QPushButton* btn[6]; // 6个按钮
    QPushButton* btnExit; // 退出小窗按钮
    void applyStyle();
private slots:
    void onButton1Clicked();
    void onButton2Clicked();
    void onButton3Clicked();
    void onButton4Clicked();
    void onButton5Clicked();
    void onButton6Clicked();
};

#endif // SUBWINDOW_H
