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

protected:
	// 重写关闭事件：直接关闭小窗时退出程序
	void closeEvent(QCloseEvent* event) override {
		Q_UNUSED(event);
		qApp->quit(); // 退出整个应用程序
	}

private:
	QPushButton* btn[6]; // 6个按钮
	QPushButton* btnExit; // 退出小窗按钮
};

#endif // SUBWINDOW_H