#ifndef COMMTEST_QT_H
#define COMMTEST_QT_H

#include "ui_CommTest_Qt.h"
#include "SubMainWindow.h"
#include "SimulationPlatform.h"
#include "RegisterTableManager.h"
#include "ScriptManager.h"
#include "Config/ConfigManager.h"
#include "MainWorkFlow.h"

#include <QtWidgets/QMainWindow>
#include <QColor>
#include <QTimer>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>
#include <QButtonGroup>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QFrame>

QT_BEGIN_NAMESPACE
namespace Ui { class CommTest_QtClass; };
QT_END_NAMESPACE

class CommTest_Qt : public QMainWindow
{
    Q_OBJECT

public:
    CommTest_Qt(QWidget* parent = nullptr);
    ~CommTest_Qt();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    // 初始化方法
    void InitializeMember();
    void InitialSignalConnect();
    void InitialLineEditValidator();
    void InitialAllConfigs();
    void InitialGuiStyle();

    // 协议相关
    void CreateCurrentProtocol();

    // 平台控制相关
    double GetDivisorFromPowerEdit(QLineEdit* edit, double defaultPower = 0.0);
    void OnWriteAxisDoubleWord();
    void OnWriteAxisFloat();

    // 自动写入相关
    void OnWritePosAutoEnableChanged(int state);
    void OnPlatformPositionChanged();

    // 菜单栏相关
    void OnShowAboutDialog();

    // 日志显示
    void UpdateLogDisplay(QString strNewLog);

private:
    Ui::CommTest_QtClass* ui;

    // 子窗口
    std::unique_ptr<SubMainWindow> m_subWindow;
    SimulationPlatform* m_simulationPlatform;

    // 工作流
    MainWorkFlow* m_pWorkFlow;
    std::unique_ptr<MainWorkFlow::IBaseController> m_PlatformController;
    std::unique_ptr<CommBase::CommInfoBase> m_CurInfo;

    // 配置管理
    ConfigManager* m_configManager;

    // 寄存器表格管理器
    std::unique_ptr<RegisterTableManager> m_registerTableManager;

    // 脚本管理器
    std::unique_ptr<ScriptManager> m_scriptManager;

    // 表格闪烁相关
    QMap<QTableWidgetItem*, QTimer*> m_animationTimers;
    QMap<QTableWidgetItem*, QString> m_lastTextValues;

    // 日志显示状态
    int m_nLogStat;

    // 自动写入相关
    double m_lastRealTimeX;
    double m_lastRealTimeY;
    double m_lastRealTimeAngle;
    double m_lastBaseX;
    double m_lastBaseY;
    double m_lastBaseAngle;
    QTimer* m_positionCheckTimer;
};

#endif // COMMTEST_QT_H
