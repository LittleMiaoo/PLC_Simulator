#ifndef COMMTEST_QT_H
#define COMMTEST_QT_H

#include "ui_CommTest_Qt.h"
#include "ScriptEditor.h"

#include "SubMainWindow.h"
#include "MainWorkFlow.h"
#include "SimulationPlatform.h"
#include "Config/ConfigManager.h"

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

#define REGISTER_TABLE_COLUMN_COUNT 10
#define REGISTER_TABLE_ROW_COUNT	21

QT_BEGIN_NAMESPACE
namespace Ui { class CommTest_QtClass; };
QT_END_NAMESPACE

class CommTest_Qt : public QMainWindow
{
	Q_OBJECT

public:
	CommTest_Qt(QWidget* parent = nullptr);
	~CommTest_Qt();

	//void InitialSocketPort();
private:

	void InitializeMember();

	void InitialSignalConnect();

	void InitialLuaScript();

	// 辅助函数：根据脚本索引打开编辑器
	void OpenScriptEditor(int scriptIndex);

	// 平台控制相关槽函数
	void OnMovePlatformAbsInt32(int32_t x, int32_t y, int32_t angle);
	void OnMovePlatformAbsFloat(double x, double y, double angle);
	void OnMovePlatformRelativeInt32(int32_t x, int32_t y, int32_t angle);
	void OnMovePlatformRelativeFloat(double x, double y, double angle);

	// 辅助函数：从控件获取幂次值并计算除数
	double GetDivisorFromPowerEdit(QLineEdit* edit, double defaultPower = 0.0);

	// 轴位置写入相关槽函数
	void OnWriteAxisDoubleWord();
	void OnWriteAxisFloat();

	// 菜单栏相关槽函数
	void OnShowAboutDialog();

	void InitRegisterTable();
	void UpdateTableInfo(int nStart,bool bInitialize = false);

	void InitialLineEditValidator();    //20251024  wm  输入框输入内容限制初始化

	void InitialAllConfigs();
	
	void InitialGuiStyle();             // 添加界面样式设置函数

	Ui::CommTest_QtClass* ui;

	std::unique_ptr<SubMainWindow> m_subWindow; // 小窗口实例
    //std::unique_ptr<SimulationPlatform> m_simulationPlatform;   // 模拟平台窗口实例
	SimulationPlatform* m_simulationPlatform;
	ConfigManager* m_configManager;
private:

	MainWorkFlow* m_pWorkFlow;
	//CommSocket::SocketCommInfo* m_CurInfo;
	std::unique_ptr<CommBase::CommInfoBase> m_CurInfo; //内存自动管理

	//列表刷新相关
	QMap<QTableWidgetItem*, QTimer*> m_animationTimers;
	QMap<QTableWidgetItem*, QString> m_lastTextValues;

	void CreateCurrentProtocol();
	//QMap<ProtocolType,QString> m_ProtocolTypeMap;

	int m_nLogStat;
	int m_nIntStat;

	//切换数据显示模式时更新表格数据


	//定义vector存储当前表格能显示的数据
    std::vector<DataTypeConvert> m_vecRegisterVal;

	//存储当前打开的脚本编辑器指针（单实例）
	ScriptEditor* m_pCurrentScriptEditor;
	//当前编辑器关联的脚本索引
	int m_nCurrentScriptIndex;

	//定义从MainWorkFlow中获取寄存器数据的函数
    void GetRegisterVals(int nStart);

	//定义设置寄存器数据的函数
    void SetRegisterVals(int nStart);
   
	//传入QTableWidgetItem*指针,更新寄存器数据的函数
    void UpdateRegisterVals(QTableWidgetItem* pItem);

	//单元格输入内容的合法性判断
    bool CheckInput(QTableWidgetItem* pItem);

	//数据类型选择为字符串时,单元格输入内容的合法性判断
    bool CheckInput_str(QTableWidgetItem* pItem, const QString& text);

	//数据类型选择为单字/双字时,单元格输入内容的合法性判断
    bool CheckInput_int(QTableWidgetItem* pItem, const QString& text,int32_t MinVal,int32_t MaxVal);

	//数据类型选择为浮点数时,单元格输入内容的合法性判断
    bool CheckInput_float(QTableWidgetItem* pItem, const QString& text,double MinVal,double MaxVal);

	//数据类型选择为单字/双字且十六进制时,单元格输入内容的合法性判断
    bool CheckInput_int_Hex(QTableWidgetItem* pItem, const RegisterDataType& type);

	//将m_vecRegisterVal中每个元素,根据ui->cmbBox_DataType的类型显示到表格中
    void DisplayRegisterVals();

	//显示Char8类型数据
    void DisplayRegisterVals_Char8();

	//显示Int16类型数据
    void DisplayRegisterVals_Int16();

    //显示Int32类型数据
    void DisplayRegisterVals_Int32();

    //显示Float类型数据
    void DisplayRegisterVals_Float();

    //显示Double类型数据
    void DisplayRegisterVals_Double();


};

#endif