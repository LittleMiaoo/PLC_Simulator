#include "SubMainWindow.h"
#include <QCoreApplication>

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

    connect(btn[0], &QPushButton::clicked, this, &SubMainWindow::onButton1Clicked);
    connect(btn[1], &QPushButton::clicked, this, &SubMainWindow::onButton2Clicked);
    connect(btn[2], &QPushButton::clicked, this, &SubMainWindow::onButton3Clicked);
    connect(btn[3], &QPushButton::clicked, this, &SubMainWindow::onButton4Clicked);
    connect(btn[4], &QPushButton::clicked, this, &SubMainWindow::onButton5Clicked);
    connect(btn[5], &QPushButton::clicked, this, &SubMainWindow::onButton6Clicked);

    gridLayout->setContentsMargins(8,8,8,8);
    gridLayout->setHorizontalSpacing(8);
    gridLayout->setVerticalSpacing(8);

    applyStyle();
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

void SubMainWindow::onButton1Clicked()
{
    //QString p = QCoreApplication::applicationDirPath() + "/Config/LuaScript/script1.lua";
    emit executeLuaScript(1);
}

void SubMainWindow::onButton2Clicked()
{
    //QString p = QCoreApplication::applicationDirPath() + "/Config/LuaScript/script2.lua";
    emit executeLuaScript(2);
}

void SubMainWindow::onButton3Clicked()
{
    //QString p = QCoreApplication::applicationDirPath() + "/Config/LuaScript/script3.lua";
    emit executeLuaScript(3);
}

void SubMainWindow::onButton4Clicked()
{
   // QString p = QCoreApplication::applicationDirPath() + "/Config/LuaScript/script4.lua";
    emit executeLuaScript(4);
}

void SubMainWindow::onButton5Clicked()
{
    //QString p = QCoreApplication::applicationDirPath() + "/Config/LuaScript/script5.lua";
    emit executeLuaScript(5);
}

void SubMainWindow::onButton6Clicked()
{
    //QString p = QCoreApplication::applicationDirPath() + "/Config/LuaScript/script6.lua";
    emit executeLuaScript(6);
}

void SubMainWindow::applyStyle()
{
    const QString buttonStyleSheet =
        "QPushButton {"
        "    background-color: #F0F0F0;"
        "    color: #212121;"
        "    border: 1px solid #CCCCCC;"
        "    border-radius: 6px;"
        "    padding: 6px 12px;"
        "    font-weight: 500;"
        "    font-size: 11pt;"
        "    outline: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #E8E8E8;"
        "    border: 1px solid #4CA3E0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #D0D0D0;"
        "    border: 1px solid #2E7BA8;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #E0E0E0;"
        "    color: #808080;"
        "    border: 1px solid #DDDDDD;"
        "}";

    for (int i = 0; i < 6; ++i) {
        if (btn[i]) btn[i]->setStyleSheet(buttonStyleSheet);
    }
    if (btnExit) btnExit->setStyleSheet(buttonStyleSheet);
}
