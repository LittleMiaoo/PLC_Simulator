#include "QuickPanel.h"
#include <QCoreApplication>
#include <QFrame>
#include <QCloseEvent>
#include <QEvent>
QuickPanel::QuickPanel(QWidget* parent)
    : QDialog(parent)
{
	// 注:窗口标志(独立窗口、隐藏最小/最大/关闭按钮)在主窗口创建小窗处统一设置

	// 初始化布局（所有按钮垂直排列：6个脚本按钮竖排，退出按钮置底）
	QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 创建脚本按钮并垂直添加;点击发 executeLuaScript(脚本索引 0-based)
    constexpr int kScriptButtonCount = 6;
    for (int i = 0; i < kScriptButtonCount; ++i)
    {
        auto* button = new QPushButton(this);
        mainLayout->addWidget(button);
        btn.push_back(button);

        connect(button, &QPushButton::clicked, this, [this, i]() {
            emit executeLuaScript(i);
        });
    }

    // 脚本按钮与退出按钮之间:间距 + 细分隔线,明确区分两类操作
    mainLayout->addSpacing(4);
    QFrame* separator = new QFrame(this);
    separator->setObjectName("hSeparator");
    mainLayout->addWidget(separator);
    mainLayout->addSpacing(4);

    // 创建退出小窗的按钮（置于底部）
    btnExit = new QPushButton(tr("退出小窗"), this);
    mainLayout->addWidget(btnExit);

	// 退出按钮点击：隐藏小窗并显示主窗口（通过信号通知主窗口）
    connect(btnExit, &QPushButton::clicked, this, [this]() {
        this->hide();       // 隐藏小窗
        emit showMainWindow(); // 发射信号，通知主窗口显示
        });

    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // 最小宽度:保证标题栏文字(含版本号与"子窗口")完整显示
    setMinimumWidth(340);

    // 按钮样式由全局主题样式表(ThemeManager)统一控制,不再设置局部样式
}

void QuickPanel::setButtonTexts(const QStringList& texts)
{
    for (int i = 0; i < btn.size() && i < texts.size(); ++i)
        btn[i]->setText(texts[i]);
}

void QuickPanel::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
    qApp->quit(); // 退出整个应用程序
}

void QuickPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange && btnExit)
        btnExit->setText(tr("退出小窗"));
    QDialog::changeEvent(event);
}