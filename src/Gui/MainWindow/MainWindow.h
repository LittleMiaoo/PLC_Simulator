/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
#include "QuickPanel.h"
#include "Core/PlatformTypes.h"
#include "RegisterTableController.h"
#include "ScriptManager.h"
#include "StatusBarController.h"
#include "Core/PlatformController.h"
#include "AuxDialogs.h"
#include "Config/ConfigStore.h"
#include "MainWorkflow.h"
#include "Theme/ThemeManager.h"
#include "I18n/LanguageManager.h"

#include <QtWidgets/QMainWindow>
#include <QEvent>
#include <QMenu>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; };
class QAction;
class QLineEdit;
class QLabel;
class QToolBar;
class QPushButton;
QT_END_NAMESPACE

class SimulationPlatform;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    // 初始化阶段(构造函数按此顺序调用)
    void createMembers();          // ① 创建所有成员对象
    void setupUiContent();         // ② 填充静态 UI(下拉框 / 只读 / 状态栏)
    void loadConfigs();            // ③ 读取持久化配置
    void setupInputValidators();   // ④ 输入校验(须晚于 loadConfigs)
    void applyThemePref();         // ⑤ 应用持久化主题(须先于建菜单)
    void connectSignals();         // ⑥ 连接所有信号槽
    void initialRefresh();         // ⑦ 首屏刷新表格

    // 信号连接分组(由 connectSignals 调用)
    void buildMenus();
    void connectWindowSignals();
    void connectRegisterTable();
    void connectComm();
    void connectLog();
    void connectScript();
    void connectStatusBar();

    // 协议相关
    void CreateCurrentProtocol();

    // 通信连接开/关(由 Btn_Create 分发)
    void openConnection();
    void closeConnection();
    void setCommControlsEnabled(bool enabled);   // 未连接态可编辑通信参数+按钮"打开",连接态锁定+"关闭"

    // 平台控制相关
    void OnWriteAxisDoubleWord();
    void OnWriteAxisFloat();

    // 轴写入辅助
    bool axisStartAddrValid(int addr) const;
    void writeAxisPos(int startAddr, PlatformController::NumFormat fmt, Platform which);
    void writeAxisManual(PlatformController::NumFormat fmt);

    // 自动写入相关
    void OnPlatformPoseChanged(Platform which, const Pose& pose);

    // 平台参数相关
    void refreshAxisAddrStatus();
    void applyPlatformParams();    // 把 m_platformParams 应用到控制器+状态栏(加载/编辑复用)

    // 寄存器表显示设置持久化(起始地址 / 数据类型 / 进制)
    void saveRegisterView();

    // 菜单栏相关
    void OnThemeSelected(Theme theme);
    void OnLanguageSelected(AppLanguage lang);
    void retranslateDynamicUi();

    // 工具栏图标按当前主题前景色重染(初次构建 + 主题切换时调用)
    void updateToolbarIcons();

    // 日志显示
    void UpdateLogDisplay(QString strNewLog);


    // 脚本名称编辑框辅助
    QVector<QLineEdit*> scriptNameEdits() const;

private:
    Ui::MainWindow* ui;

    // 平台控制:参数数据成员
    AuxDialogs::PlatformParams m_platformParams{3, 3, 114, 120};

    // 平台菜单 actions(跨槽访问的才做成员;仅菜单构建期用的为局部变量)
    QAction* m_actShowPlatform = nullptr;
    QAction* m_actAutoWrite = nullptr;
    QAction* m_actFmtFloat = nullptr;
    QAction* m_actFmtInt32 = nullptr;

    // 手动写入工具栏按钮(主题切换时需重染图标,故持为成员)
    QAction* m_actManualFloat = nullptr;
    QAction* m_actManualInt32 = nullptr;

    // 菜单栏(语言切换时需重设文字)
    QMenu* m_helpMenu = nullptr;
    QAction* m_actAbout = nullptr;
    QAction* m_actChangelog = nullptr;
    QMenu* m_viewMenu = nullptr;
    QAction* m_actLightTheme = nullptr;
    QAction* m_actDarkTheme = nullptr;
    QMenu* m_platformMenu = nullptr;
    QMenu* m_themeMenu = nullptr;
    QMenu* m_langMenu = nullptr;
    QAction* m_actLangZh = nullptr;
    QAction* m_actLangEn = nullptr;
    QMenu* m_fmtMenu = nullptr;
    QAction* m_actPlatformParams = nullptr;
    QToolBar* m_platformToolBar = nullptr;

    // 子窗口
    std::unique_ptr<QuickPanel> m_subWindow;
    SimulationPlatform* m_simulationPlatform;

    // 工作流
    MainWorkflow* m_pWorkflow;
    std::unique_ptr<PlatformController> m_platformController;

    // 配置管理
    ConfigStore* m_configStore;

    // 寄存器表格管理器
    std::unique_ptr<RegisterTableController> m_registerTableController;

    // 脚本管理器
    std::unique_ptr<ScriptManager> m_scriptManager;

    // 脚本名称编辑框缓存(运行期不变,scriptNameEdits() 首次发现后填充)
    mutable QVector<QLineEdit*> m_scriptNameEdits;

    // 状态栏控制器
    std::unique_ptr<StatusBarController> m_statusBarController;


    // UI 就绪标志:startup 期间控件初值变更不落盘,initialRefresh 末尾置真
    bool m_uiReady = false;

    // 日志显示格式(通信日志按此在 ASCII / HEX 间切换)
    enum class LogFormat { Ascii, Hex };
    LogFormat m_logFormat = LogFormat::Ascii;

    // 通信日志重复帧过滤状态(与上一帧方向+端点+数据相同则不刷屏;收/发各自独立)
    QString    m_lastRecEndpoint;
    QByteArray m_lastRecData;
    QString    m_lastSendEndpoint;
    QByteArray m_lastSendData;
};

#endif // MAINWINDOW_H
