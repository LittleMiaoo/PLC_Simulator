#include "SubMainWindow.h"

#ifdef _WIN32
#ifdef _DEBUG
#include "MemoryLeakDetector.h"
#endif
#endif

SubMainWindow::SubMainWindow(QWidget* parent)
	: QDialog(parent) 
{
	//setWindowFlags(windowFlags() | Qt::Tool);

	// 初始化布局（3列2行放6个按钮，第3行放退出按钮）
	QGridLayout* gridLayout = new QGridLayout(this);

	// 创建6个按钮并添加到网格布局
	for (int i = 0; i < 6; ++i) 
	{
		btn[i] = new QPushButton(this);
		// 布局位置：第i/3行，第i%3列（0-2列，0-1行）
		gridLayout->addWidget(btn[i], i / 3, i % 3);
	}

	// 创建退出小窗的按钮（跨3列显示）
	btnExit = new QPushButton("退出小窗", this);
	gridLayout->addWidget(btnExit, 2, 0, 1, 3); // 第2行，0列，占1行3列

	// 退出按钮点击：隐藏小窗并显示主窗口（通过信号通知主窗口）
	connect(btnExit, &QPushButton::clicked, this, [this]() {
		this->hide();       // 隐藏小窗
		emit showMainWindow(); // 发射信号，通知主窗口显示
		});
}

void SubMainWindow::setButtonTexts(const QStringList& texts)
{
	for (int i = 0; i < 6; ++i) 
	{
		if (i < texts.size()) 
		{
			btn[i]->setText(texts[i]);
		}
	}
}