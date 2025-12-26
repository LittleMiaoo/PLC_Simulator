#include "CommTest_Qt.h"
#include "../version.h"
#include <QDir>
#include <QFile>
#include <cmath>
#include<QWindow>


CommTest_Qt::CommTest_Qt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CommTest_QtClass())
    , m_pWorkFlow(nullptr)
    , m_simulationPlatform(nullptr)
    , m_configManager(nullptr)
    , m_nLogStat(0)
{
    ui->setupUi(this);
    setWindowTitle(QString("%1 - v%2").arg(APP_NAME).arg(APP_VERSION));

    // 初始化成员实例
    InitializeMember();

    // 加载配置
    InitialAllConfigs();

    // 初始化信号槽连接
    InitialSignalConnect();

    // 初始化界面样式
    InitialGuiStyle();

    // 初始化寄存器表格管理器
    m_registerTableManager = std::make_unique<RegisterTableManager>(
        ui->table_RegisterData,
        ui->cmbBox_DataType,
        ui->edit_RegisterAddr,
        m_pWorkFlow,
        this
    );
    m_registerTableManager->initTable();
    ui->table_RegisterData->installEventFilter(this);

    // 初始化脚本管理器
    m_scriptManager = std::make_unique<ScriptManager>(m_pWorkFlow, this);
    m_scriptManager->initScriptExecution();

    // 连接脚本执行和编辑按钮
    m_scriptManager->connectExecuteButton(0, ui->Btn_Execute_1);
    m_scriptManager->connectExecuteButton(1, ui->Btn_Execute_2);
    m_scriptManager->connectExecuteButton(2, ui->Btn_Execute_3);
    m_scriptManager->connectExecuteButton(3, ui->Btn_Execute_4);
    m_scriptManager->connectExecuteButton(4, ui->Btn_Execute_5);
    m_scriptManager->connectExecuteButton(5, ui->Btn_Execute_6);

    m_scriptManager->connectEditButton(1, ui->Btn_Edit_1);
    m_scriptManager->connectEditButton(2, ui->Btn_Edit_2);
    m_scriptManager->connectEditButton(3, ui->Btn_Edit_3);
    m_scriptManager->connectEditButton(4, ui->Btn_Edit_4);
    m_scriptManager->connectEditButton(5, ui->Btn_Edit_5);
    m_scriptManager->connectEditButton(6, ui->Btn_Edit_6);

    m_scriptManager->connectLoopCheckBox(0, ui->ChkBox_LoopEnable_1);
    m_scriptManager->connectLoopCheckBox(1, ui->ChkBox_LoopEnable_2);
    m_scriptManager->connectLoopCheckBox(2, ui->ChkBox_LoopEnable_3);
    m_scriptManager->connectLoopCheckBox(3, ui->ChkBox_LoopEnable_4);
    m_scriptManager->connectLoopCheckBox(4, ui->ChkBox_LoopEnable_5);
    m_scriptManager->connectLoopCheckBox(5, ui->ChkBox_LoopEnable_6);

    // 初始化输入限制
    InitialLineEditValidator();

	// 在状态栏添加作者和版本信息
	const QString datetime = QStringLiteral("%1 %2").arg(APP_COMPILE_DATE).arg(APP_COMPILE_TIME);

	QLabel *label = new QLabel(this);
    label->setText(QStringLiteral("Author:%1 Version:%2 Compile Time: %3")
		.arg(APP_AUTHOR)
		.arg(APP_VERSION)
		.arg(datetime));
	ui->statusBar->addPermanentWidget(label);


	// 更新表格显示
	m_registerTableManager->updateTableInfo(ui->edit_RegisterAddr->text().toUInt(), true);

	struct SimulationPlatformController: public MainWorkFlow::IBaseController
	{
		CommTest_Qt* m_pParent;
		explicit SimulationPlatformController(CommTest_Qt* parent) : m_pParent(parent) {}
		void MovePlatformAbsFloat(double dX, double dY, double dAngle) override
		{
			if (m_pParent && m_pParent->m_simulationPlatform)
			{
				m_pParent->m_simulationPlatform->SetRealTimePlatformAbs(dX, dY, dAngle);
			}
		}
		void MovePlatformRelativeFloat(double dX, double dY, double dAngle) override
		{
			if (m_pParent && m_pParent->m_simulationPlatform)
			{
				m_pParent->m_simulationPlatform->SetRealTimePlatformRelative(dX, dY, dAngle);
			}
		}
		void MovePlatformRelativeInt32(int32_t nX, int32_t nY, int32_t nAngle) override
		{
			if (m_pParent && m_pParent->m_simulationPlatform)
			{
				// 获取除数：10的幂次方
				double divisorXY = m_pParent->GetDivisorFromPowerEdit(m_pParent->ui->edit_Unit_XY);
				double divisorD = m_pParent->GetDivisorFromPowerEdit(m_pParent->ui->edit_Unit_D);

				// 转换坐标值：除以10的幂次方
				double convertedX = static_cast<double>(nX) / divisorXY;
				double convertedY = static_cast<double>(nY) / divisorXY;
				double convertedAngle = static_cast<double>(nAngle) / divisorD;

				// 控制平台移动
				m_pParent->m_simulationPlatform->SetRealTimePlatformRelative(convertedX, convertedY, convertedAngle);
			}
		}
		void MovePlatformAbsInt32(int32_t nX, int32_t nY, int32_t nAngle) override
		{
			if (m_pParent && m_pParent->m_simulationPlatform)
			{
				// 获取除数：10的幂次方
				double divisorXY = m_pParent->GetDivisorFromPowerEdit(m_pParent->ui->edit_Unit_XY);
				double divisorD = m_pParent->GetDivisorFromPowerEdit(m_pParent->ui->edit_Unit_D);

				// 转换坐标值：除以10的幂次方
				double convertedX = static_cast<double>(nX) / divisorXY;
				double convertedY = static_cast<double>(nY) / divisorXY;
				double convertedAngle = static_cast<double>(nAngle) / divisorD;

				// 控制平台移动
				m_pParent->m_simulationPlatform->SetRealTimePlatformAbs(convertedX, convertedY, convertedAngle);
			}
		}

		void GetCurrentPosInt32(int32_t& nX, int32_t& nY, int32_t& nAngle) override
		{
			if (m_pParent && m_pParent->m_simulationPlatform)
			{
				double x = 0.0, y = 0.0, angle = 0.0;
				m_pParent->m_simulationPlatform->GetRealTimePlatformData(x, y, angle);

				// 获取幂次值并计算乘数（注意：这里是乘以幂次，与平台控制时除以幂次相反）
				double multiplierXY = m_pParent->GetDivisorFromPowerEdit(m_pParent->ui->edit_Unit_XY);  // 10^powerXY
				double multiplierD = m_pParent->GetDivisorFromPowerEdit(m_pParent->ui->edit_Unit_D);    // 10^powerD

				// 将坐标值乘以10的幂次方并转换为int32
				 nX = static_cast<int32_t>(x * multiplierXY);
				 nY = static_cast<int32_t>(y * multiplierXY);
				 nAngle = static_cast<int32_t>(angle * multiplierD);
			}
		}
		void GetCurrentPosFloat(double& dX, double& dY, double& dAngle) override
		{
			if (m_pParent && m_pParent->m_simulationPlatform)
			{
				m_pParent->m_simulationPlatform->GetRealTimePlatformData(dX, dY, dAngle);
			}
		}
	};
	m_PlatformController = std::make_unique<SimulationPlatformController>(this);

	if (m_pWorkFlow == nullptr)return;
	m_pWorkFlow->SetBaseController(m_PlatformController.get());
}

CommTest_Qt::~CommTest_Qt()
{
	// 先关闭并释放模拟平台
	if (m_simulationPlatform != nullptr)
	{
		m_simulationPlatform->close();
		delete m_simulationPlatform;
		m_simulationPlatform = nullptr;
	}

	// 释放MainWorkFlow单例
	MainWorkFlow::ReleaseWorkFlow();

	delete ui;
}

void CommTest_Qt::InitialAllConfigs()
{
	// 加载之前保存的配置
	if (m_configManager)
	{
		m_configManager->LoadAllConfigs();

		//加载通信信息
		{
			std::unique_ptr<CommConfig> commInfo;
			if (m_configManager->LoadCommInfo(commInfo))
			{
				if (commInfo != nullptr && commInfo->type == CommBase::CommType::eSocket)
				{
					ui->edit_IP->setText(commInfo->params["ip"].toString());
					ui->edit_Port->setText(commInfo->params["port"].toString());
				}
			}
		}

		// 应用加载的配置到UI
		// 加载脚本名称
		QStringList scriptNames;
		if (m_configManager->LoadScriptNames(scriptNames) && scriptNames.size() == 6)
		{
			ui->edit_ScriptName_1->setText(scriptNames[0]);
			ui->edit_ScriptName_2->setText(scriptNames[1]);
			ui->edit_ScriptName_3->setText(scriptNames[2]);
			ui->edit_ScriptName_4->setText(scriptNames[3]);
			ui->edit_ScriptName_5->setText(scriptNames[4]);
			ui->edit_ScriptName_6->setText(scriptNames[5]);
		}

		// 加载协议类型
		int protocolType = -1;
		if (m_configManager->LoadProtocolType(protocolType) && protocolType >= 0)
		{
			//根据protocolType遍历ui->cmbBox_ProtocolType查找对应索引并设置
			for (int i = 0; i <  ui->cmbBox_ProtocolType->count(); ++i)
			{
				QVariant var = ui->cmbBox_ProtocolType->itemData(i);
				if (var.isValid() && var.canConvert<ProtocolType>())
				{
					ProtocolType type = var.value<ProtocolType>();
					if (static_cast<int>(type) == protocolType)
					{
						ui->cmbBox_ProtocolType->setCurrentIndex(i);
						CreateCurrentProtocol();
						break;
					}
				}
			}
		}

		// 加载模拟平台参数
		double markCenterDistance = 0.0, screenRatio = 0.0;
		if (m_configManager->LoadSimulationPlatformParams(markCenterDistance, screenRatio))
		{
			if (m_simulationPlatform != nullptr)
				m_simulationPlatform->SetSimulationPlatformParams(markCenterDistance, screenRatio);
		}
	}
}


void CommTest_Qt::InitializeMember()
{
	m_configManager = new ConfigManager(this);

	if (m_pWorkFlow == nullptr)
	{
		m_pWorkFlow = MainWorkFlow::InitialWorkFlow(this);
	}

	// 初始化小窗口
	m_subWindow = std::make_unique<SubMainWindow>();
	//将当前窗口的名称设置为小窗名称
	m_subWindow->setWindowTitle(this->windowTitle() + " - 子窗口");
	m_subWindow->setWindowFlags(Qt::Window); // 设置为独立窗口
    m_subWindow->createWinId();
    //初始化模拟平台窗口
    m_simulationPlatform = new SimulationPlatform(this);
	//将当前窗口的名称设置为模拟平台窗口名称
	m_simulationPlatform->setWindowTitle(this->windowTitle() + " - 模拟平台");
	m_simulationPlatform->setWindowFlags(
		Qt::Dialog
		| Qt::WindowMinimizeButtonHint  // 显示最小化按钮
		| Qt::WindowMaximizeButtonHint  // 显示最大化按钮
		| Qt::WindowCloseButtonHint     // 显示关闭按钮
		| Qt::WindowSystemMenuHint      // 保留系统菜单（支持右键最小化/最大化）
	);
	m_simulationPlatform->setAttribute(Qt::WA_ShowWithoutActivating, true);

    // 监听所有窗口状态变化
    connect(windowHandle(), &QWindow::windowStateChanged, this, [this](Qt::WindowState state) {
        // 主窗口状态变化
        if (this->isVisible()) {
            // 只有主窗口显示时才同步
            m_simulationPlatform->setWindowState(state);
        }
    });

    // 监听子窗口状态变化
	auto subHandle = m_subWindow->windowHandle();
    if (subHandle)
	{
        connect(subHandle, &QWindow::windowStateChanged, this, [this](Qt::WindowState state) {
            // 子窗口状态变化
            if (m_subWindow->isVisible()) {
                // 只有子窗口显示时才同步
                m_simulationPlatform->setWindowState(state);
            }
			m_subWindow->activateWindow();
        });
    }

	//协议设置相关
	{
		QMap<ProtocolType, QString> m_ProtocolTypeMap;
		m_ProtocolTypeMap[ProtocolType::eProRegKeyencePCLink] = "基恩士PC-LINK上位链路协议";
		m_ProtocolTypeMap[ProtocolType::eProRegMitsubishiQBinary] = "三菱MC协议二进制通信";

		for (auto it = m_ProtocolTypeMap.begin(); it != m_ProtocolTypeMap.end(); ++it)
		{
			ui->cmbBox_ProtocolType->addItem(it.value(), QVariant::fromValue(it.key()));
		}

		if (ui->cmbBox_ProtocolType->currentIndex() < 0)
		{
			ui->cmbBox_ProtocolType->setCurrentIndex(0);
		}
		else
		{
			CreateCurrentProtocol();
		}
	}

	//ui->cmbBox_DataType 数据类型显示转换
	{
		QMap<RegisterDataType, QString> DataType;
		DataType[RegisterDataType::eDataTypeChar8] = "字符";
        DataType[RegisterDataType::eDataTypeInt16] = "单字";
        DataType[RegisterDataType::eDataTypeInt32] = "双字";
        DataType[RegisterDataType::eDataTypeFloat] = "单精度";
        DataType[RegisterDataType::eDataTypeDouble] = "双精度";

		//绑定到comboBox
        for (auto it = DataType.begin(); it != DataType.end(); ++it)
		{
			ui->cmbBox_DataType->addItem(it.value(), QVariant::fromValue(it.key()));
		}

		ui->cmbBox_DataType->setCurrentIndex(0);
	}

	//数据格式显示相关
	{
		QButtonGroup* group1 = new QButtonGroup(this);
		QButtonGroup* group2 = new QButtonGroup(this);

		group1->addButton(ui->Radio_Data_DEC);
		group1->addButton(ui->Radio_Data_HEX);
		ui->Radio_Data_DEC->setChecked(true);

		group2->addButton(ui->Radio_Log_Ascii);
		group2->addButton(ui->Radio_Log_HEX);
		ui->Radio_Log_Ascii->setChecked(true);

		connect(group1, &QButtonGroup::buttonToggled, this, [=](QAbstractButton* button, bool checked) {
			if (checked)
			{
				qDebug() << "组1中选中了:" << button->text();
				if (button == ui->Radio_Data_DEC)
				{
					m_registerTableManager->setIntDisplayStat(0);
				}
				else if (button == ui->Radio_Data_HEX)
				{
					m_registerTableManager->setIntDisplayStat(1);
				}
				m_registerTableManager->setShouldFlash(false);
				const QSignalBlocker blocker(ui->table_RegisterData);

				m_registerTableManager->updateTableInfo(ui->edit_RegisterAddr->text().toUInt());

				m_registerTableManager->setShouldFlash(true);
			}
			});

		connect(group2, &QButtonGroup::buttonToggled, this, [=](QAbstractButton* button, bool checked) {
			if (checked)
			{

				if (button == ui->Radio_Log_Ascii)
				{
					m_nLogStat = 0;
				}
				else if (button == ui->Radio_Log_HEX)
				{
					m_nLogStat = 1;
				}

				qDebug() << "日志组中选中了:" << button->text() << "m_nLogStat = " << m_nLogStat;
			}
			});
	}

}

void CommTest_Qt::InitialSignalConnect()
{
	// 初始化菜单栏
	QMenu* helpMenu = ui->menuBar->addMenu("帮助(&H)");
	QAction* aboutAction = helpMenu->addAction("关于(&A)");
	connect(aboutAction, &QAction::triggered, this, &CommTest_Qt::OnShowAboutDialog);

    // 连接小窗口的显示主窗口信号到主窗口的show()槽
	connect(m_subWindow.get(), &SubMainWindow::showMainWindow, this, &CommTest_Qt::show);

    connect(m_subWindow.get(), &SubMainWindow::executeLuaScript, this, [=](int buttonId) {
        if (m_pWorkFlow == nullptr) return;
        int idx = buttonId;
        QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += QString("LuaFile%1.lua").arg(idx);
        QFile f(strLuaPath);
        if (!f.exists()) {
            QMessageBox::critical(this, "Lua执行错误", QString("脚本不存在: %1").arg(strLuaPath));
			UpdateLogDisplay(QString("脚本不存在: %1").arg(strLuaPath));
            return;
        }
        try {
            if (!m_pWorkFlow->RunLuaScript(idx-1, strLuaPath)) { // buttonId从1开始，索引从0开始
                QMessageBox::critical(this, "Lua执行错误", QString("执行失败: %1").arg(strLuaPath));
                UpdateLogDisplay(QString("执行Lua脚本失败: %1").arg(strLuaPath));
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Lua执行异常", QString("%1").arg(e.what()));
            UpdateLogDisplay(QString("Lua执行异常: %1").arg(e.what()));
        } catch (...) {
            QMessageBox::critical(this, "Lua执行异常", "未知异常");
            UpdateLogDisplay("Lua执行异常: 未知异常");
        }
    });

	// 连接显示/隐藏模拟平台窗口按钮
	connect(ui->Btn_ShowPlatform, &QPushButton::clicked, this, [=]() {
		if (m_simulationPlatform->isVisible()) {
			m_simulationPlatform->hide();
		} else {
			m_simulationPlatform->show();
		}
	});

	// 连接轴位置写入按钮
	connect(ui->Btn_WriteAxisDoubleWord, &QPushButton::clicked, this, &CommTest_Qt::OnWriteAxisDoubleWord);
	connect(ui->Btn_WriteAxisFloat, &QPushButton::clicked, this, &CommTest_Qt::OnWriteAxisFloat);

	// 隐藏主窗口槽函数
	connect(ui->Btn_HideMainWindow, &QPushButton::clicked, this, [=]() {
		QStringList lineEditTexts;
		lineEditTexts << ui->edit_ScriptName_1->text()
			<< ui->edit_ScriptName_2->text()
			<< ui->edit_ScriptName_3->text()
			<< ui->edit_ScriptName_4->text()
			<< ui->edit_ScriptName_5->text()
			<< ui->edit_ScriptName_6->text();

		// 设置小窗口6个按钮的文本
		if(m_subWindow != nullptr)
		{
			m_subWindow->setButtonTexts(lineEditTexts);

			// 隐藏主窗口，显示小窗口
			this->hide();

			// 设置小窗口为工具窗口，不会单独占用任务栏图标
			m_subWindow->show();
			m_subWindow->activateWindow();
		}

	});

	//寄存器表格相关信号
	{
		// 表格闪烁提示槽函数
		connect(ui->table_RegisterData, &QTableWidget::itemChanged, this, [=](QTableWidgetItem* item) {
			if (!item) return;
			if (!m_registerTableManager->shouldFlash()) return;

			//忽略奇数列的变化(地址列)
			if (item->column() % 2 == 0)	return;

			// 检查文本是否真的改变了
			QString currentText = item->text();
			QString lastText = m_lastTextValues.value(item);

			if (currentText == lastText) {
				return; // 文本没有实际改变，忽略
			}

			// 更新保存的文本值
			m_lastTextValues[item] = currentText;

			// 取消该item可能存在的未完成动画
			if (m_animationTimers.contains(item))
			{
				QTimer* existingTimer = m_animationTimers.value(item);
				existingTimer->stop();
				existingTimer->deleteLater();
				m_animationTimers.remove(item);
			}


			// 设置高亮颜色
			item->setBackground(QColor(255, 100, 100));

			// 创建定时器用于恢复颜色
			QTimer* restoreTimer = new QTimer(this);
			restoreTimer->setSingleShot(true);

			connect(restoreTimer, &QTimer::timeout, this, [=]() {
				if (item)
				{
					item->setBackground(QBrush());
				}

				// 清理定时器
				if (m_animationTimers.contains(item)) {
					m_animationTimers.remove(item);
				}
				restoreTimer->deleteLater();
			});

			m_animationTimers[item] = restoreTimer;

			restoreTimer->start(400);	// 启动定时器，400ms后执行
		});

		QAbstractItemDelegate* delegate = ui->table_RegisterData->itemDelegate();

		// 连接表格单元格输入完成信号槽
		connect(delegate, &QAbstractItemDelegate::commitData, this, [this](QWidget* editor) {
			// 这个信号在数据提交时触发，可以获取到正确的单元格
			QModelIndex currentIndex = ui->table_RegisterData->currentIndex();
			QTableWidgetItem* currentItem = ui->table_RegisterData->item(currentIndex.row(), currentIndex.column());
			if(currentItem != nullptr)
			{
				m_registerTableManager->updateRegisterVals(currentItem);
			}
		});
	}

	// 切换协议
	connect(ui->cmbBox_ProtocolType, &QComboBox::currentIndexChanged, this, [this](int index) {
		// 创建协议并保存协议类型
		CreateCurrentProtocol();
		if (m_configManager && index >= 0) {
			ProtocolType selectedType = ui->cmbBox_ProtocolType->currentData().value<ProtocolType>();
			m_configManager->SaveProtocolType(static_cast<int>(selectedType));
		}
	});

	// 点击打开连接按钮
	connect(ui->Btn_Create, &QPushButton::clicked, this, [=] {

		if (m_pWorkFlow == nullptr)	return;

		if (m_pWorkFlow->IsCommOpen())
		{
			if (!m_pWorkFlow->CloseComm())
			{
				UpdateLogDisplay("关闭连接失败!");
				return;
			}

			ui->Btn_Create->setText("打开链接");
			ui->edit_IP->setEnabled(true);
			ui->edit_Port->setEnabled(true);
		}
		else
		{
			CommConfig cfg;
			cfg.type = CommBase::CommType::eSocket;
			cfg.params.insert("ip", ui->edit_IP->text());
			cfg.params.insert("port", ui->edit_Port->text().toUShort());
			cfg.params.insert("listenNum", 10);
			cfg.params.insert("socketType", 0);
			m_pWorkFlow->ConfigureComm(cfg);

			if (!m_pWorkFlow->OpenComm())
			{
				UpdateLogDisplay("打开连接失败!");
				return;
			}
			if (m_configManager)
			{
				m_configManager->SaveCommInfo(&cfg);
			}
			auto ExecuteRequest = [this](const QByteArray& in, QByteArray& out) {
				if (!m_pWorkFlow) return false;
				return m_pWorkFlow->ProcessRequest(in, out);
			};
			m_pWorkFlow->SetRequestProcessor(ExecuteRequest);

			ui->Btn_Create->setText("关闭链接");
			ui->edit_IP->setEnabled(false);
			ui->edit_Port->setEnabled(false);
		}
	});

	// 点击清除寄存器按钮
	connect(ui->Btn_ClearRegister, &QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;
		m_registerTableManager->setShouldFlash(false);
		m_pWorkFlow->ResetAllRegisters(0);
		m_registerTableManager->setShouldFlash(true);
	});

	// 点击清除日志
	connect(ui->Btn_ClearCommLog, &QPushButton::clicked, this, [=] {
		ui->text_CommLog->clear();
	});

	// 修改显示寄存器地址
	connect(ui->edit_RegisterAddr, &QLineEdit::textChanged, this, [=](const QString& text) {
		if (text == "")	return;

		int nAddr = text.toInt();

		if (nAddr < 0)	return;

		// 从工作流获取寄存器数据
		m_registerTableManager->getRegisterVals(nAddr);

		m_registerTableManager->setShouldFlash(false);
		const QSignalBlocker blocker(ui->table_RegisterData);

		m_registerTableManager->updateTableInfo(nAddr);

		m_registerTableManager->setShouldFlash(true);

	});

	// 修改显示寄存器数据类型
	connect(ui->cmbBox_DataType, &QComboBox::currentIndexChanged, this, [=]{
		m_registerTableManager->setShouldFlash(false);
		const QSignalBlocker blocker(ui->table_RegisterData);
		m_registerTableManager->updateTableInfo(ui->edit_RegisterAddr->text().toUInt());
		m_registerTableManager->setShouldFlash(true);

	});

	// 主控类持有的通信实例信号转发
	if (m_pWorkFlow != nullptr)
	{
		// 通信日志记录
		connect(m_pWorkFlow, &MainWorkFlow::commLogRecord, this, [=](QString strLogInfo) {
			UpdateLogDisplay(strLogInfo);
		});

		// 接收数据
		connect(m_pWorkFlow, &MainWorkFlow::dataReceived, this, [=](QString objectInfo, QByteArray recData) {

			QString strdata(recData);

			if (1 == m_nLogStat)
			{
				strdata = QString(recData.toHex().toUpper());
			}

			UpdateLogDisplay(objectInfo + strdata);

		});

		// 发送数据
		connect(m_pWorkFlow, &MainWorkFlow::dataSend, this, [=](QString objectInfo, QByteArray sendData) {

			QString strdata(sendData);

			if (1 == m_nLogStat)
			{
				strdata = QString(sendData.toHex().toUpper());
			}

			UpdateLogDisplay(objectInfo + strdata);

			});

		// 寄存器数据改变
		connect(m_pWorkFlow, &MainWorkFlow::RegisterDataUpdate, this, [=] {
			m_registerTableManager->updateTableInfo(ui->edit_RegisterAddr->text().toUInt());
		});
	}

	// 脚本名称编辑框自动保存事件
	if (m_configManager)
	{
		auto saveScriptNames = [this]() {
			QStringList names;
			names << ui->edit_ScriptName_1->text()
				  << ui->edit_ScriptName_2->text()
				  << ui->edit_ScriptName_3->text()
				  << ui->edit_ScriptName_4->text()
				  << ui->edit_ScriptName_5->text()
				  << ui->edit_ScriptName_6->text();
			m_configManager->SaveScriptNames(names);
		};

		connect(ui->edit_ScriptName_1, &QLineEdit::textChanged, this, saveScriptNames);
		connect(ui->edit_ScriptName_2, &QLineEdit::textChanged, this, saveScriptNames);
		connect(ui->edit_ScriptName_3, &QLineEdit::textChanged, this, saveScriptNames);
		connect(ui->edit_ScriptName_4, &QLineEdit::textChanged, this, saveScriptNames);
		connect(ui->edit_ScriptName_5, &QLineEdit::textChanged, this, saveScriptNames);
		connect(ui->edit_ScriptName_6, &QLineEdit::textChanged, this, saveScriptNames);
	}

	// 初始化SimulationPlatform自动保存参数
	connect(m_simulationPlatform, &SimulationPlatform::parametersChanged, this, [=](double markCenterDistance, double screenRatio) {
		if (m_configManager)
		{
			m_configManager->SaveSimulationPlatformParams(markCenterDistance, screenRatio);
		}
	});
}

void CommTest_Qt::InitialLineEditValidator()
{
	QIntValidator* PortValid = new QIntValidator(0,65535,this);
	QIntValidator* RegisterShowAddr = new QIntValidator(0,
		REGISTER_VAL_NUM - 1 - REGISTER_TABLE_COLUMN_COUNT * REGISTER_TABLE_ROW_COUNT / 2, this);
	QIntValidator* UnitXYD = new QIntValidator(1, 20, this);

	QIntValidator* AxisRegisterAddr = new QIntValidator(0, REGISTER_VAL_NUM - 1 - 6, this);
	//只能输入整型数
	ui->edit_Port->setValidator(PortValid);
	ui->edit_RegisterAddr->setValidator(RegisterShowAddr);
	ui->edit_Unit_XY->setValidator(UnitXYD);
	ui->edit_Unit_D->setValidator(UnitXYD);
	ui->edit_AxisPosRegisterAddr->setValidator(AxisRegisterAddr);

	ui->edit_IP->setInputMask("000.000.000.000;"); // IP地址格式
}

void CommTest_Qt::CreateCurrentProtocol()
{
	int nCurIndex = ui->cmbBox_ProtocolType->currentIndex();

	if (nCurIndex < 0) return;

	QVariant data = ui->cmbBox_ProtocolType->itemData(nCurIndex);

	if (m_pWorkFlow == nullptr)return;

	m_pWorkFlow->CreateCommProtocol(data.value<ProtocolType>());
}

// ====================平台控制相关槽函数实现====================

// 辅助函数：从控件获取幂次值并计算10的幂次方作为除数
double CommTest_Qt::GetDivisorFromPowerEdit(QLineEdit* edit, double defaultPower)
{
	if (edit == nullptr)
	{
		return std::pow(10.0, defaultPower);
	}

	bool ok = false;
	double power = edit->text().toDouble(&ok);

	if (!ok)
	{
		power = defaultPower;
	}

	return std::pow(10.0, power);
}

// ====================轴位置写入相关槽函数实现====================

void CommTest_Qt::OnWriteAxisDoubleWord()
{
	if (m_simulationPlatform == nullptr || m_pWorkFlow == nullptr)
	{
		return;
	}

	// 获取平台实时位置
	double x = 0.0, y = 0.0, angle = 0.0;
	m_simulationPlatform->GetRealTimePlatformData(x, y, angle);

	// 获取幂次值并计算乘数（注意：这里是乘以幂次，与平台控制时除以幂次相反）
	double multiplierXY = GetDivisorFromPowerEdit(ui->edit_Unit_XY);  // 10^powerXY
	double multiplierD = GetDivisorFromPowerEdit(ui->edit_Unit_D);    // 10^powerD

	// 将坐标值乘以10的幂次方并转换为int32
	int32_t xInt32 = static_cast<int32_t>(x * multiplierXY);
	int32_t yInt32 = static_cast<int32_t>(y * multiplierXY);
	int32_t angleInt32 = static_cast<int32_t>(angle * multiplierD);

	// 获取对应的寄存器地址
	bool ok = false;
	int startAddr = ui->edit_AxisPosRegisterAddr->text().toInt(&ok);
	if (!ok)
	{
		UpdateLogDisplay("错误: 轴位置地址无效");
		return;
	}

	// 计算需要的数组索引
	// 每个DataTypeConvert包含4个int16，可以存储2个int32
	// X占用2个int16（地址startAddr和startAddr+1）
	// Y占用2个int16（地址startAddr+2和startAddr+3）
	// Angle占用2个int16（地址startAddr+4和startAddr+5）
	if (startAddr >= REGISTER_VAL_NUM - 6)
	{
		UpdateLogDisplay("错误: 寄存器地址超出范围");
		return;
	}

	DataTypeConvert data;
	data.u_Int32[0] = xInt32;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);
	data.u_Int32[0] = yInt32;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);
	data.u_Int32[0] = angleInt32;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);

	UpdateLogDisplay(QString("轴位置双字写入成功: X=%1, Y=%2, Angle=%3 (地址:%4)")
		.arg(xInt32).arg(yInt32).arg(angleInt32).arg(startAddr));

	m_registerTableManager->updateTableInfo(ui->edit_RegisterAddr->text().toUInt());
}

void CommTest_Qt::OnWriteAxisFloat()
{
	if (m_simulationPlatform == nullptr || m_pWorkFlow == nullptr)
	{
		return;
	}

	// 获取平台实时位置
	double x = 0.0, y = 0.0, angle = 0.0;
	m_simulationPlatform->GetRealTimePlatformData(x, y, angle);

	// 转换为float（直接使用，不需要乘幂次）
	float xFloat = static_cast<float>(x);
	float yFloat = static_cast<float>(y);
	float angleFloat = static_cast<float>(angle);

	// 获取对应的寄存器地址
	bool ok = false;
	int startAddr = ui->edit_AxisPosRegisterAddr->text().toInt(&ok);
	if (!ok)
	{
		UpdateLogDisplay("错误: 轴位置地址无效");
		return;
	}

	if (startAddr >= REGISTER_VAL_NUM - 6)
	{
		UpdateLogDisplay("错误: 寄存器地址超出范围");
		return;
	}

	DataTypeConvert data;
	data.u_float[0] = xFloat;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);
	data.u_float[0] = yFloat;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);
	data.u_float[0] = angleFloat;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);

	UpdateLogDisplay(QString("轴位置浮点写入成功: X=%1, Y=%2, Angle=%3 (地址:%4)")
		.arg(xFloat).arg(yFloat).arg(angleFloat).arg(startAddr));

	m_registerTableManager->updateTableInfo(ui->edit_RegisterAddr->text().toUInt());
}

// ====================菜单栏相关槽函数实现====================

void CommTest_Qt::OnShowAboutDialog()
{
	QDialog aboutDialog(this);
	aboutDialog.setWindowTitle(QString("关于 %1").arg(APP_NAME));
	aboutDialog.setFixedSize(400, 320);
	aboutDialog.setWindowFlags(aboutDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout* mainLayout = new QVBoxLayout(&aboutDialog);
	mainLayout->setSpacing(15);
	mainLayout->setContentsMargins(30, 25, 30, 20);

	// 图标显示（居中）
	QLabel* iconLabel = new QLabel(&aboutDialog);
	QPixmap iconPixmap(":/CommTest_Qt/PLC_Simulator.ico");
	iconLabel->setPixmap(iconPixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	iconLabel->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(iconLabel);

	// 应用名称（居中）
	QLabel* nameLabel = new QLabel(APP_NAME, &aboutDialog);
	nameLabel->setAlignment(Qt::AlignCenter);
	nameLabel->setStyleSheet("font-size: 18pt; font-weight: bold; color: #333333;");
	mainLayout->addWidget(nameLabel);

	// 版本信息（居中）
	QString compileDate = QString::fromLatin1(APP_COMPILE_DATE);
	QString compileTime = QString::fromLatin1(APP_COMPILE_TIME);
	QString versionInfo = QString("Version: %1\nCompile Time: %2 %3\nAuthor: %4")
		.arg(APP_VERSION)
		.arg(compileDate)
		.arg(compileTime)
		.arg(APP_AUTHOR);
	QLabel* versionLabel = new QLabel(versionInfo, &aboutDialog);
	versionLabel->setAlignment(Qt::AlignCenter);
	versionLabel->setStyleSheet("font-size: 10pt; color: #666666;");
	mainLayout->addWidget(versionLabel);

	// 分隔线
	QFrame* line = new QFrame(&aboutDialog);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	line->setStyleSheet("background-color: #CCCCCC;");
	mainLayout->addWidget(line);

	// 应用描述（靠左）
	QLabel* descLabel = new QLabel(APP_DESCRIPTION, &aboutDialog);
	descLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	descLabel->setWordWrap(true);
	descLabel->setStyleSheet("font-size: 10pt; color: #333333;");
	mainLayout->addWidget(descLabel);

	mainLayout->addStretch();

	// 确定按钮（居中）
	QPushButton* okButton = new QPushButton("确定", &aboutDialog);
	okButton->setFixedSize(80, 30);
	okButton->setStyleSheet(
		"QPushButton {"
		"    background-color: #4CA3E0;"
		"    color: white;"
		"    border: none;"
		"    border-radius: 4px;"
		"    font-size: 10pt;"
		"}"
		"QPushButton:hover {"
		"    background-color: #3A8BC8;"
		"}"
		"QPushButton:pressed {"
		"    background-color: #2E7BA8;"
		"}"
	);
	connect(okButton, &QPushButton::clicked, &aboutDialog, &QDialog::accept);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addStretch();
	mainLayout->addLayout(buttonLayout);

	aboutDialog.exec();
}

void CommTest_Qt::InitialGuiStyle()
{
	// 设置应用程序浅色系主题
	QPalette darkPalette;

	// 设置窗口和面板背景色为浅色
	darkPalette.setColor(QPalette::Window, QColor(245, 245, 245));           // 浅灰白色背景
	darkPalette.setColor(QPalette::WindowText, QColor(33, 33, 33));         // 深灰色文字
	darkPalette.setColor(QPalette::Base, QColor(255, 255, 255));            // 白色输入框背景
	darkPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));   // 交替背景色
	darkPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));     // 提示框背景
	darkPalette.setColor(QPalette::ToolTipText, QColor(33, 33, 33));        // 提示框文字
	darkPalette.setColor(QPalette::Text, QColor(33, 33, 33));               // 文本颜色
	darkPalette.setColor(QPalette::Button, QColor(240, 240, 240));          // 按钮背景色
	darkPalette.setColor(QPalette::ButtonText, QColor(33, 33, 33));         // 按钮文字
	darkPalette.setColor(QPalette::BrightText, QColor(255, 255, 255));      // 亮色文字
	darkPalette.setColor(QPalette::Highlight, QColor(76, 163, 224));        // 高亮色（蓝色）
	darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));  // 高亮文字
	darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));  // 禁用文字
	darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));  // 禁用按钮文字

	// 应用调色板
	QApplication::setPalette(darkPalette);

	// 定义通用的按钮样式表 - 带圆角、阴影和悬停效果
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
		"}"
		"";

	// 应用按钮样式到所有主要按钮
	if (ui->Btn_Create != nullptr) ui->Btn_Create->setStyleSheet(buttonStyleSheet);
	if (ui->Btn_HideMainWindow != nullptr) ui->Btn_HideMainWindow->setStyleSheet(buttonStyleSheet);
	if (ui->Btn_ShowPlatform != nullptr) ui->Btn_ShowPlatform->setStyleSheet(buttonStyleSheet);
	if (ui->Btn_ClearRegister != nullptr) ui->Btn_ClearRegister->setStyleSheet(buttonStyleSheet);
	if (ui->Btn_ClearCommLog != nullptr) ui->Btn_ClearCommLog->setStyleSheet(buttonStyleSheet);
	if (ui->Btn_WriteAxisDoubleWord != nullptr) ui->Btn_WriteAxisDoubleWord->setStyleSheet(buttonStyleSheet);
	if (ui->Btn_WriteAxisFloat != nullptr) ui->Btn_WriteAxisFloat->setStyleSheet(buttonStyleSheet);

	// 应用样式到Lua脚本相关的按钮
	for (int i = 1; i <= 6; ++i)
	{
		QPushButton* executeBtn = this->findChild<QPushButton*>(QString("Btn_Execute_%1").arg(i));
		QPushButton* editBtn = this->findChild<QPushButton*>(QString("Btn_Edit_%1").arg(i));

		if (executeBtn != nullptr) {
			executeBtn->setStyleSheet(buttonStyleSheet);
		}
		if (editBtn != nullptr) {
			editBtn->setStyleSheet(buttonStyleSheet);
		}
	}

	// 定义输入框和组合框的样式表
	const QString inputStyleSheet =
		"QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {"
		"    background-color: #FFFFFF;"
		"    color: #212121;"
		"    border: 1px solid #CCCCCC;"
		"    border-radius: 4px;"
		"    padding: 4px 6px;"
		"    font-size: 10pt;"
		"}"
		"QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {"
		"    border: 2px solid #4CA3E0;"
		"    background-color: #FFFEF5;"
		"}"
		"QComboBox::drop-down {"
		"    border: none;"
		"}"
		"QComboBox::down-arrow {"
		"    image: url(:/qt/etc/images/down_arrow.png);"
		"}"
		"";

	// 应用输入框样式
	if (ui->edit_IP != nullptr) ui->edit_IP->setStyleSheet(inputStyleSheet);
	if (ui->edit_Port != nullptr) ui->edit_Port->setStyleSheet(inputStyleSheet);
	if (ui->edit_RegisterAddr != nullptr) ui->edit_RegisterAddr->setStyleSheet(inputStyleSheet);
	if (ui->edit_Unit_XY != nullptr) ui->edit_Unit_XY->setStyleSheet(inputStyleSheet);
	if (ui->edit_Unit_D != nullptr) ui->edit_Unit_D->setStyleSheet(inputStyleSheet);
	if (ui->edit_AxisPosRegisterAddr != nullptr) ui->edit_AxisPosRegisterAddr->setStyleSheet(inputStyleSheet);

	for (int i = 1; i <= 6; ++i)
	{
		QLineEdit* scriptNameEdit = this->findChild<QLineEdit*>(QString("edit_ScriptName_%1").arg(i));
		if (scriptNameEdit != nullptr) {
			scriptNameEdit->setStyleSheet(inputStyleSheet);
		}
	}

	// 应用组合框样式
	if (ui->cmbBox_ProtocolType != nullptr) ui->cmbBox_ProtocolType->setStyleSheet(inputStyleSheet);
	if (ui->cmbBox_DataType != nullptr) ui->cmbBox_DataType->setStyleSheet(inputStyleSheet);

	// 定义文本编辑框和表格的样式表
	const QString textEditStyleSheet =
		"QTextEdit, QPlainTextEdit {"
		"    background-color: #FFFFFF;"
		"    color: #212121;"
		"    border: 1px solid #CCCCCC;"
		"    border-radius: 4px;"
		"    padding: 4px;"
		"    font-family: 'Courier New', monospace;"
		"    font-size: 10pt;"
		"}"
		"";

	if (ui->text_CommLog != nullptr) ui->text_CommLog->setStyleSheet(textEditStyleSheet);
	//日志控件只读
	ui->text_CommLog->setReadOnly(true);

	// 定义表格样式
	const QString tableStyleSheet =
		"QTableWidget {"
		"    background-color: #FFFFFF;"
		"    alternate-background-color: #F5F5F5;"
		"    gridline-color: #DDDDDD;"
		"    border: 1px solid #CCCCCC;"
		"    border-radius: 4px;"
		"}"
		"QTableWidget::item {"
		"    padding: 2px;"
		"    color: #212121;"
		"}"
		"QTableWidget::item:selected {"
		"    background-color: #4CA3E0;"
		"    color: #FFFFFF;"
		"}"
		"QHeaderView::section {"
		"    background-color: #F0F0F0;"
		"    color: #212121;"
		"    padding: 4px;"
		"    border: 1px solid #CCCCCC;"
		"    border-radius: 0px;"
		"}"
		"";

	if (ui->table_RegisterData != nullptr) ui->table_RegisterData->setStyleSheet(tableStyleSheet);


	// 定义复选框和单选按钮的样式表
	const QString checkboxStyleSheet =
		"QCheckBox, QRadioButton {"
		"    color: #212121;"
		"    spacing: 6px;"
		"    font-size: 10pt;"
		"}"
		"QCheckBox::indicator, QRadioButton::indicator {"
		"    width: 16px;"
		"    height: 16px;"
		"    border: 1px solid #CCCCCC;"
		"    border-radius: 3px;"
		"    background-color: #FFFFFF;"
		"}"
		"QCheckBox::indicator:checked, QRadioButton::indicator:checked {"
		"    background-color: #4CA3E0;"
		"    border: 1px solid #2E7BA8;"
		"}"
		"QCheckBox::indicator:hover, QRadioButton::indicator:hover {"
		"    border: 1px solid #4CA3E0;"
		"}"
		"";

	// 应用复选框样式到所有复选框
	for (int i = 1; i <= 6; ++i)
	{
		QCheckBox* loopCheckBox = this->findChild<QCheckBox*>(QString("ChkBox_LoopEnable_%1").arg(i));
		if (loopCheckBox != nullptr) {
			loopCheckBox->setStyleSheet(checkboxStyleSheet);
		}
	}

	// 应用单选按钮样式
	if (ui->Radio_Data_DEC != nullptr) ui->Radio_Data_DEC->setStyleSheet(checkboxStyleSheet);
	if (ui->Radio_Data_HEX != nullptr) ui->Radio_Data_HEX->setStyleSheet(checkboxStyleSheet);
	if (ui->Radio_Log_Ascii != nullptr) ui->Radio_Log_Ascii->setStyleSheet(checkboxStyleSheet);
	if (ui->Radio_Log_HEX != nullptr) ui->Radio_Log_HEX->setStyleSheet(checkboxStyleSheet);

	// 设置菜单栏样式
	const QString menuBarStyleSheet =
		"QMenuBar {"
		"    background-color: #F0F0F0;"
		"    color: #212121;"
		"    border-bottom: 1px solid #CCCCCC;"
		"}"
		"QMenuBar::item:selected {"
		"    background-color: #E8E8E8;"
		"}"
		"QMenu {"
		"    background-color: #FFFFFF;"
		"    color: #212121;"
		"    border: 1px solid #CCCCCC;"
		"}"
		"QMenu::item:selected {"
		"    background-color: #4CA3E0;"
		"    color: #FFFFFF;"
		"}"
		"";

	if (ui->menuBar != nullptr) ui->menuBar->setStyleSheet(menuBarStyleSheet);
}

bool CommTest_Qt::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui->table_RegisterData)
	{
		return QMainWindow::eventFilter(watched, event);
	}
	return QMainWindow::eventFilter(watched, event);
}

void CommTest_Qt::UpdateLogDisplay(QString strNewLog)
{
	QTextDocument *document = ui->text_CommLog->document();

	//获取当前行数
	int lineCount = document->blockCount();
	//如果行数超过最大限制，则清空
	int nMaxLogLines = 5000;
	if (lineCount > nMaxLogLines)
	{
		ui->text_CommLog->clear();
	}

	//添加新日志，自动滚动到最底部
	ui->text_CommLog->append(strNewLog);
	ui->text_CommLog->moveCursor(QTextCursor::End);
}
