#include "CommTest_Qt.h"
#include <QDir>
#include <QFile>
#include <cmath>


CommTest_Qt::CommTest_Qt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CommTest_QtClass())
{
    ui->setupUi(this);
	
	m_CurInfo = nullptr;
	m_pWorkFlow = nullptr;
	m_subWindow = nullptr;
	
	m_simulationPlatform = nullptr;
	m_configManager = new ConfigManager(this);
	// 初始化脚本编辑器指针
	m_pCurrentScriptEditor = nullptr;
	m_nCurrentScriptIndex = -1;
	
	m_nLogStat = 0;
	m_nIntStat = 0;
	//初始化m_vecRegisterVal
	int dataCellCount = REGISTER_TABLE_ROW_COUNT * (REGISTER_TABLE_COLUMN_COUNT / 2);
	int convertCount = (dataCellCount + 3) / 4;  // 向上取整到4的倍数

	m_vecRegisterVal.resize(convertCount);

	//初始化成员实例
	InitializeMember();
	
	//m_pWorkFlow->CreateCommProtocol(ProtocolType::eProRegMitsubishiQBinary);
	InitialAllConfigs();

	//初始化信号槽连接
	InitialSignalConnect();
	

	//初始化界面样式
	InitialGuiStyle();
	
	//初始化表格显示
	InitRegisterTable();

	//初始化输入限制
	InitialLineEditValidator();

	//更新表格显示
	UpdateTableInfo(ui->edit_RegisterAddr->text().toUInt(),true);

	//int* thisint = new int(1);

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
		CommBase::CommInfoBase* commInfo = nullptr;
		if (m_configManager->LoadCommInfo(commInfo))
		{
			if (commInfo != nullptr && commInfo->GetCommType() == CommBase::CommType::eSocket)
			{
				CommSocket::SocketCommInfo* socketInfo = dynamic_cast<CommSocket::SocketCommInfo*>(commInfo);
				ui->edit_IP->setText(socketInfo->m_strSocketIPAddress);
				ui->edit_Port->setText(socketInfo->m_nSocketPort == 0 ? "" : QString::number(socketInfo->m_nSocketPort));
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
			//ui->cmbBox_ProtocolType->setCurrentIndex(protocolType);
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
			m_simulationPlatform->SetSimulationPlatformParams(markCenterDistance, screenRatio);
		}
	}
}


void CommTest_Qt::InitializeMember()
{
	if (m_pWorkFlow == nullptr)
	{
		m_pWorkFlow = MainWorkFlow::InitialWorkFlow(this);
	}

	// 初始化小窗口
	m_subWindow = std::make_unique<SubMainWindow>();
    
    // 初始化模拟平台窗口
   // m_simulationPlatform = std::make_unique<SimulationPlatform>(this);
    m_simulationPlatform = new SimulationPlatform(this);
    m_simulationPlatform->setWindowFlags(Qt::Window); // 设置为独立窗口

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
					m_nIntStat = 0;
				}
				else if (button == ui->Radio_Data_HEX)
				{
					m_nIntStat = 1;
				}
				ui->table_RegisterData->blockSignals(true);	//修改数据进制时临时断开表格信号,以禁用闪烁提示

				UpdateTableInfo(ui->edit_RegisterAddr->text().toUInt());

                ui->table_RegisterData->blockSignals(false);
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

    //20251024	wm	 连接小窗口的显示主窗口信号到主窗口的show()槽
    connect(m_subWindow.get(), &SubMainWindow::showMainWindow, this, &CommTest_Qt::show);
    connect(m_subWindow.get(), &SubMainWindow::executeLuaScript, this, [=](int buttonId) {
        if (m_pWorkFlow == nullptr) return;
        int idx = buttonId - 1;
        QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += "LuaFile1.lua";
        QFile f(strLuaPath);
        if (!f.exists()) {
            QMessageBox::critical(this, "Lua执行错误", QString("脚本不存在: %1").arg(strLuaPath));
            ui->text_CommLog->append(QString("脚本不存在: %1").arg(strLuaPath));
            return;
        }
        try {
            if (!m_pWorkFlow->RunLuaScript(idx, strLuaPath)) {
                QMessageBox::critical(this, "Lua执行错误", QString("执行失败: %1").arg(strLuaPath));
                ui->text_CommLog->append(QString("执行Lua脚本失败: %1").arg(strLuaPath));
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Lua执行异常", QString("%1").arg(e.what()));
            ui->text_CommLog->append(QString("Lua执行异常: %1").arg(e.what()));
        } catch (...) {
            QMessageBox::critical(this, "Lua执行异常", "未知异常");
            ui->text_CommLog->append("Lua执行异常: 未知异常");
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

	//20251024	wm	隐藏主窗口槽函数
	connect(ui->Btn_HideMainWindow, &QPushButton::clicked, this, [=]() {
		QStringList lineEditTexts;
		lineEditTexts << ui->edit_ScriptName_1->text()
			<< ui->edit_ScriptName_2->text()
			<< ui->edit_ScriptName_3->text()
			<< ui->edit_ScriptName_4->text()
			<< ui->edit_ScriptName_5->text()
			<< ui->edit_ScriptName_6->text();

		// 2. 设置小窗口6个按钮的文本
		m_subWindow->setButtonTexts(lineEditTexts);

		// 3. 隐藏主窗口，显示小窗口
		this->hide();
		m_subWindow->show();
	});
	
	//寄存器表格相关信号
	{
		//20251024	wm	表格闪烁提示槽函数
		connect(ui->table_RegisterData, &QTableWidget::itemChanged, this, [=](QTableWidgetItem* item) {
			if (!item) return;

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

			//qDebug() << "检测到文本变化:" << lastText << "->" << currentText;

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
			//qDebug() << "设置高亮颜色";

			// 创建定时器用于恢复颜色
			QTimer* restoreTimer = new QTimer(this);
			restoreTimer->setSingleShot(true);

			connect(restoreTimer, &QTimer::timeout, this, [=]() {
				//qDebug() << "定时器触发，准备恢复颜色";
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

			restoreTimer->start(100);	//20251024	wm	启动定时器，500ms后执行
		});

		QAbstractItemDelegate* delegate = ui->table_RegisterData->itemDelegate();


		//连接表格单元格输入完成信号槽
		// connect(delegate, &QAbstractItemDelegate::closeEditor, this, [this](QWidget* editor, QAbstractItemDelegate::EndEditHint hint) {
		// 	QTableWidgetItem* currentItem = ui->table_RegisterData->currentItem();
		// 	//当前单元格的行列号
		// 	int row = currentItem->row();
		// 	int column = currentItem->column();
		// 	qDebug() << "当前单元格行列号:" << row << "," << column;
		// 	//判断指针有效
        //     if (currentItem)
        //     {
        //         UpdateRegisterVals(currentItem);
        //     }
		// });
		connect(delegate, &QAbstractItemDelegate::commitData, this, [this](QWidget* editor) {
			// 这个信号在数据提交时触发，可以获取到正确的单元格
			QModelIndex currentIndex = ui->table_RegisterData->currentIndex();
			 //static int commitCount = 0;
			//commitCount++;
			//qDebug() << "commitData 第" << commitCount << "次触发";
			//qDebug() << "编辑器:" << editor;
			//qDebug() << "提交数据的单元格:" << currentIndex.row() << "," << currentIndex.column();
			QTableWidgetItem* currentItem = ui->table_RegisterData->item(currentIndex.row(), currentIndex.column());
			if(currentItem != nullptr)
			{
				UpdateRegisterVals(currentItem);
			}
		});
	}

	//20251102	wm	切换协议
	connect(ui->cmbBox_ProtocolType, &QComboBox::currentIndexChanged, this, [this](int index) {
		// 创建协议并保存协议类型
		CreateCurrentProtocol();
		if (m_configManager && index >= 0) {
			ProtocolType selectedType = ui->cmbBox_ProtocolType->currentData().value<ProtocolType>();
			m_configManager->SaveProtocolType(static_cast<int>(selectedType));
		}
	});

	//20251031	wm	点击打开连接按钮
	connect(ui->Btn_Create, &QPushButton::clicked, this, [=] {

		if (m_pWorkFlow == nullptr)	return;

		if (m_pWorkFlow->IsCommOpen())
		{
			if (!m_pWorkFlow->CloseComm())
			{
				ui->text_CommLog->append("关闭连接失败!");
				return;
			}
			//m_bIsCommValid = false;

			ui->Btn_Create->setText("打开链接");
			ui->edit_IP->setEnabled(true);
			ui->edit_Port->setEnabled(true);
		}
		else
		{
			std::unique_ptr<CommSocket::SocketCommInfo> thisInfo = std::make_unique<CommSocket::SocketCommInfo>();

			thisInfo->m_strSocketIPAddress = ui->edit_IP->text();
			thisInfo->m_nSocketPort = ui->edit_Port->text().toUShort();

			m_CurInfo = std::move(thisInfo);

			m_pWorkFlow->SetCommInfo(m_CurInfo.get());

			if (!m_pWorkFlow->OpenComm())
			{
				ui->text_CommLog->append("打开连接失败!");
				return;
			}
			//m_bIsCommValid = true;
			m_configManager->SaveCommInfo(m_CurInfo.get());


			ui->Btn_Create->setText("关闭链接");
			ui->edit_IP->setEnabled(false);
			ui->edit_Port->setEnabled(false);
		}
	});

	//20251102	wm	点击清除寄存器按钮
	connect(ui->Btn_ClearRegister, &QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		m_pWorkFlow->ResetAllRegisters(0);
	});

	//20251102	wm	点击清除日志
	connect(ui->Btn_ClearCommLog, &QPushButton::clicked, this, [=] {
		ui->text_CommLog->clear();
	});

	//20251024	wm	修改显示寄存器地址
	connect(ui->edit_RegisterAddr, &QLineEdit::textChanged, this, [=](const QString& text) {
		if (text == "")	return;

		int nAddr = text.toInt();

		if (nAddr < 0)	return;

		//从MainWorkFlow中获取寄存器数据并存入m_vecRegisterVals
		GetRegisterVals(nAddr);

		ui->table_RegisterData->blockSignals(true);	//20251024	wm	修改寄存器地址时临时断开表格信号,以禁用闪烁提示

		UpdateTableInfo(nAddr);

		ui->table_RegisterData->blockSignals(false);

	});

	//20251101	wm	修改显示寄存器数据类型
    connect(ui->cmbBox_DataType, &QComboBox::currentIndexChanged, this, [=]{
		//禁用寄存器表格修改信号
   		ui->table_RegisterData->blockSignals(true);
		UpdateTableInfo(ui->edit_RegisterAddr->text().toUInt());
		ui->table_RegisterData->blockSignals(false);

	});

	//20251101	wm	主控类持有的通信实例信号转发
	if (m_pWorkFlow != nullptr)
	{
		//20251031	wm	通信日志记录
		connect(m_pWorkFlow, &MainWorkFlow::commLogRecord, this, [=](QString strLogInfo) {
			ui->text_CommLog->append(strLogInfo);
		});

		//20251031	wm	接收数据
		connect(m_pWorkFlow, &MainWorkFlow::dataReceived, this, [=](QString objectInfo, QByteArray recData) {

			QString strdata(recData);
			
			if (1 == m_nLogStat)
			{
				strdata = QString(recData.toHex().toUpper());
			}

			ui->text_CommLog->append(objectInfo + strdata);

			if (m_pWorkFlow == nullptr) return;
			//QThread::msleep(20);
			m_pWorkFlow->WorkProcess(recData);
		});

		//20251103	wm	发送数据
		connect(m_pWorkFlow, &MainWorkFlow::dataSend, this, [=](QString objectInfo, QByteArray sendData) {

			QString strdata(sendData);
			
			if (1 == m_nLogStat)
			{
				strdata = QString(sendData.toHex().toUpper());
			}
		
			ui->text_CommLog->append(objectInfo + strdata);

			});

		//20251102	wm	寄存器数据改变
		connect(m_pWorkFlow, &MainWorkFlow::RegisterDataUpdate, this, [=] {
			UpdateTableInfo(ui->edit_RegisterAddr->text().toUInt());
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

	//初始化SimulationPlatform自动保存参数
	connect(m_simulationPlatform, &SimulationPlatform::parametersChanged, this, [=](double markCenterDistance, double screenRatio) {
		if (m_configManager)
		{
			m_configManager->SaveSimulationPlatformParams(markCenterDistance, screenRatio);
		}
	});

	//初始化lua脚本相关信号槽
	InitialLuaScript();


}

void CommTest_Qt::InitialLuaScript()
{
	//连接执行lua脚本按钮
	connect(ui->Btn_Execute_1,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		//获取当前执行程序路径
		QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += "LuaFile1.lua";
		// if (!m_pWorkFlow->RunLuaScript(0,strLuaPath))
		// {
		// 	ui->text_CommLog->append("执行Lua脚本失败!");
		// }
		emit executeLuaScript(0,strLuaPath);
	});

	connect(ui->Btn_Edit_1,&QPushButton::clicked, this, [=] {
		OpenScriptEditor(1);
	});

	connect(ui->Btn_Execute_2,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += "LuaFile2.lua";
		if (!m_pWorkFlow->RunLuaScript(1,strLuaPath))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Edit_2,&QPushButton::clicked, this, [=] {
		OpenScriptEditor(2);
	});

	connect(ui->Btn_Execute_3,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += "LuaFile3.lua";
		if (!m_pWorkFlow->RunLuaScript(2,strLuaPath))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Edit_3,&QPushButton::clicked, this, [=] {
		OpenScriptEditor(3);
	});

	connect(ui->Btn_Execute_4,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += "LuaFile4.lua";
		if (!m_pWorkFlow->RunLuaScript(3,strLuaPath))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Edit_4,&QPushButton::clicked, this, [=] {
		OpenScriptEditor(4);
	});

	connect(ui->Btn_Execute_5,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += "LuaFile5.lua";
		if (!m_pWorkFlow->RunLuaScript(4,strLuaPath))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Edit_5,&QPushButton::clicked, this, [=] {
		OpenScriptEditor(5);
	});

	connect(ui->Btn_Execute_6,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += "LuaFile6.lua";
		if (!m_pWorkFlow->RunLuaScript(5,strLuaPath))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Edit_6,&QPushButton::clicked, this, [=] {
		OpenScriptEditor(6);
	});

	connect(ui->ChkBox_LoopEnable_1,&QCheckBox::stateChanged, this, [=](int state) {
		if (m_pWorkFlow == nullptr)	return;

		LuaScript* pLua = m_pWorkFlow->GetLuaScript(0);
		if (pLua != nullptr)
		{
			pLua->SetLoopValid(state == Qt::Checked);
		}
	});

	connect(ui->ChkBox_LoopEnable_2,&QCheckBox::stateChanged, this, [=](int state) {
		if (m_pWorkFlow == nullptr)	return;

		LuaScript* pLua = m_pWorkFlow->GetLuaScript(1);
		if (pLua != nullptr)
		{
			pLua->SetLoopValid(state == Qt::Checked);
		}
	});

	connect(ui->ChkBox_LoopEnable_3,&QCheckBox::stateChanged, this, [=](int state) {
		if (m_pWorkFlow == nullptr)	return;

		LuaScript* pLua = m_pWorkFlow->GetLuaScript(2);
		if (pLua != nullptr)
		{
			pLua->SetLoopValid(state == Qt::Checked);
		}
	});

	connect(ui->ChkBox_LoopEnable_4,&QCheckBox::stateChanged, this, [=](int state) {
		if (m_pWorkFlow == nullptr)	return;

		LuaScript* pLua = m_pWorkFlow->GetLuaScript(3);
		if (pLua != nullptr)
		{
			pLua->SetLoopValid(state == Qt::Checked);
		}
	});

	connect(ui->ChkBox_LoopEnable_5,&QCheckBox::stateChanged, this, [=](int state) {
		if (m_pWorkFlow == nullptr)	return;

		LuaScript* pLua = m_pWorkFlow->GetLuaScript(4);
		if (pLua != nullptr)
		{
			pLua->SetLoopValid(state == Qt::Checked);
		}
	});

	connect(ui->ChkBox_LoopEnable_6,&QCheckBox::stateChanged, this, [=](int state) {
		if (m_pWorkFlow == nullptr)	return;

		LuaScript* pLua = m_pWorkFlow->GetLuaScript(5);
		if (pLua != nullptr)
		{
			pLua->SetLoopValid(state == Qt::Checked);
		}
	});

	// 连接所有6个Lua脚本实例的平台控制信号
	for (int i = 0; i < LUA_SCRIPT_NUM; ++i)
	{
		LuaScript* pLua = m_pWorkFlow->GetLuaScript(i);
		if (pLua != nullptr)
		{
			// 连接绝对位置Int32信号
			connect(pLua, &LuaScript::MovePlatformAbsInt32, this, &CommTest_Qt::OnMovePlatformAbsInt32);
			
			// 连接绝对位置Float信号
			connect(pLua, &LuaScript::MovePlatformAbsFloat, this, &CommTest_Qt::OnMovePlatformAbsFloat);
			
			// 连接相对位置Int32信号
			connect(pLua, &LuaScript::MovePlatformRelativeInt32, this, &CommTest_Qt::OnMovePlatformRelativeInt32);
			
			// 连接相对位置Float信号
			connect(pLua, &LuaScript::MovePlatformRelativeFloat, this, &CommTest_Qt::OnMovePlatformRelativeFloat);
		}
	}
}

void CommTest_Qt::OpenScriptEditor(int scriptIndex)
{
	if (m_pWorkFlow == nullptr)	return;

	// 获取可执行文件所在目录
	QString appDir = QCoreApplication::applicationDirPath();
	QString scriptPath = appDir + "/Config/LuaScript/LuaFile" + QString::number(scriptIndex) + ".lua";

	// 确保目录存在
	QDir dir(appDir + "/Config/LuaScript");
	if (!dir.exists())
	{
		dir.mkpath(".");
	}

	// 如果文件不存在，创建一个空文件
	QFile file(scriptPath);
	if (!file.exists())
	{
		if (file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream out(&file);
			out << "-- Lua Script File " << scriptIndex << "\n";
			out << "-- Write your Lua code here\n";
			file.close();
		}
	}

	// 检查是否已经有编辑器打开
	if (m_pCurrentScriptEditor != nullptr)
	{
		// 如果要打开的脚本就是当前打开的脚本，直接置于前台
		if (m_nCurrentScriptIndex == scriptIndex)
		{
			m_pCurrentScriptEditor->raise();
			m_pCurrentScriptEditor->activateWindow();
			return;
		}

		// 弹出确认对话框
		QMessageBox msgBox(this);
		msgBox.setWindowTitle("确认切换脚本");
		msgBox.setText("是否保存当前脚本更改并切换到新的脚本？");
		msgBox.setIcon(QMessageBox::Question);
		
		QPushButton* saveButton = msgBox.addButton("是", QMessageBox::YesRole);
		QPushButton* discardButton = msgBox.addButton("否", QMessageBox::NoRole);
		QPushButton* cancelButton = msgBox.addButton("取消", QMessageBox::RejectRole);
		
		msgBox.exec();
		
		QAbstractButton* clickedButton = msgBox.clickedButton();
		
		if (clickedButton == cancelButton)
		{
			// 用户选择取消，不做任何操作
			return;
		}
		else if (clickedButton == saveButton)
		{
			// 用户选择保存，调用编辑器的保存函数
			QString currentScriptPath = appDir + "/Config/LuaScript/LuaFile" + QString::number(m_nCurrentScriptIndex) + ".lua";
			QFile currentFile(currentScriptPath);
			if (currentFile.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				QTextStream out(&currentFile);
				out.setEncoding(QStringConverter::Utf8);
				out << m_pCurrentScriptEditor->getScriptContent();
				currentFile.close();
			}
		}
		// else: 用户选择"否"，直接丢弃当前更改

		// 关闭当前编辑器
		m_pCurrentScriptEditor->close();
		//m_pCurrentScriptEditor->deleteLater();
		delete m_pCurrentScriptEditor;
		m_pCurrentScriptEditor = nullptr;
		m_nCurrentScriptIndex = -1;
	}

	// 创建新的编辑器窗口
	ScriptEditor* pScriptEditor = new ScriptEditor(this, m_pWorkFlow->GetLuaScript(scriptIndex - 1));
	pScriptEditor->setAttribute(Qt::WA_DeleteOnClose); // 确保窗口关闭时会被删除
	pScriptEditor->setScriptName(scriptPath);

	// 加载文件内容
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream in(&file);
		pScriptEditor->loadScript(in.readAll());
		file.close();
	}

	// 保存编辑器指针
	m_pCurrentScriptEditor = pScriptEditor;
	m_nCurrentScriptIndex = scriptIndex;

	// 连接关闭信号，清理指针
	connect(pScriptEditor, &QObject::destroyed, this, [=]() {
		m_pCurrentScriptEditor = nullptr;
		m_nCurrentScriptIndex = -1;
	});

	pScriptEditor->show();
}

void CommTest_Qt::InitRegisterTable()
{
	ui->table_RegisterData->setColumnCount(REGISTER_TABLE_COLUMN_COUNT);
	ui->table_RegisterData->setRowCount(REGISTER_TABLE_ROW_COUNT);

	QTableWidgetItem* Item;
	QString strItemInfo;
	for (int i = 0 ; i < REGISTER_TABLE_COLUMN_COUNT;i++)
	{
		strItemInfo = i % 2 ? "值" : "地址";
		Item = new QTableWidgetItem(strItemInfo); 

		ui->table_RegisterData->setColumnWidth(i, 80);
		ui->table_RegisterData->setHorizontalHeaderItem(i, Item); //设置水平表头单元格的Item
	}

	//QHeaderView* item = ui->table_RegisterData->verticalHeader();
	//item->setHidden(true);
	for (int i = 0 ; i < REGISTER_TABLE_ROW_COUNT;i++)
	{
		strItemInfo = " ";
		Item = new QTableWidgetItem(strItemInfo); 
		ui->table_RegisterData->setVerticalHeaderItem(i, Item); //设置垂直表头单元格的Item
		//禁用单元格tab
		//ui->table_RegisterData->setTabKeyNavigation(false);
	}

	for (int row = 0; row < REGISTER_TABLE_ROW_COUNT; ++row)
	{
		for (int col = 0; col < REGISTER_TABLE_COLUMN_COUNT; ++col)
		{
			strItemInfo = "";
			Item = new QTableWidgetItem(strItemInfo); 
			Item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			ui->table_RegisterData->setItem(row, col, Item);
		}
	}

	ui->table_RegisterData->setAlternatingRowColors(true); // 启用交替行颜色功能

	// 设置样式表：指定正常行和交替行的背景色
	ui->table_RegisterData->setStyleSheet(
		"QTableWidget {"
		"   background-color: rgb(255, 255, 255); /* 默认行背景色 */"
		"   alternate-background-color: rgb(240, 240, 240); /* 交替行背景色 */"
		"   gridline-color: rgb(200, 200, 200); /* 网格线颜色 */"
		"}"
	);

}

void CommTest_Qt::UpdateTableInfo(int nStart,bool bInitialize /* = false */)
{
	const int rowCount = ui->table_RegisterData->rowCount();    // 行数
	const int colCount = ui->table_RegisterData->columnCount(); // 列数
	const int step = rowCount;									// 列组间隔步长

	int nRegisterNum = m_pWorkFlow->GetRegisterNum();

	for (int row = 0; row < rowCount; ++row) 
	{
		for (int col = 0; col < colCount; col += 2) //遍历偶数列(显示地址)
		{
			QTableWidgetItem* item = ui->table_RegisterData->item(row, col);

			int group = col / 2;
			int addrNum = nStart + row + group * step; // 计算地址数字
			item->setText(QString("D%1").arg(addrNum, 5, 10, QChar('0'))); // 显示格式：D100
			item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 设为只读
			
			ui->table_RegisterData->setItem(row, col, item);
		}
	}

	GetRegisterVals(ui->edit_RegisterAddr->text().toUInt());
	DisplayRegisterVals();
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

	m_pWorkFlow->CreateCommProtocol(data.value<ProtocolType>());

}

void CommTest_Qt::GetRegisterVals(int nStart)
{
    for (int i = 0 ; i < m_vecRegisterVal.size();i++)
    {
		for (int j = 0 ; j < 4;j++)
		{
			m_vecRegisterVal.at(i).u_Int16[j] = m_pWorkFlow->GetRegisterVal(nStart + i * 4 + j);
		}
    }
}

void CommTest_Qt::SetRegisterVals(int nStart)
{
    for (int i = 0 ; i < m_vecRegisterVal.size();i++)
    {
		for (int j = 0 ; j < 4;j++)
		{
			m_pWorkFlow->SetRegisterVal(nStart + i * 4 + j, m_vecRegisterVal.at(i).u_Int16[j]);
		}
    }
}

void CommTest_Qt::UpdateRegisterVals(QTableWidgetItem* pItem)
{
	QTableWidgetItem* currentItem = ui->table_RegisterData->currentItem();

	//判断指针是否为空
	if (pItem == nullptr) return;

	CheckInput(pItem);


	int nRow = pItem->row();
	int nCol = pItem->column();

	if (nCol % 2 == 0) return;	//偶数列为地址列,不处理


	int rowCount = ui->table_RegisterData->rowCount();    // 行数

	int nStart = ui->edit_RegisterAddr->text().toUInt();

	int group = (nCol - 1) / 2; // 对应地址列的组索引
	int addr = nStart + nRow + group * rowCount; // 当前修改数据对应的地址

	//主界面m_vecRegisterVal暂存当前修改的数据
	int ndataIndex = nRow + group * rowCount;

	//每个RegisterDataType格对应4个单元格,每个单元格数据对应一个int16_t
	int nRegiseterValIndex = ndataIndex / 4;

	//判断ui->cmbBox_DataType选择类型
	int nCurIndex = ui->cmbBox_DataType->currentIndex();

	if (nCurIndex < 0) return;
	QVariant data = ui->cmbBox_DataType->itemData(nCurIndex);
	//判断data类型
	switch (data.value<RegisterDataType>())
	{
	case RegisterDataType::eDataTypeChar8:
	{
		int nCurChar = 0;
		while (nCurChar < 2 && pItem->text().length() > nCurChar)
		{
			
			//将每个字符转化为一个unicode并存储到对应的m_vecRegisterVal的uchars数组中
			int ncharIndex = ndataIndex % 8 + nCurChar;
			char nchar = pItem->text().at(nCurChar).toLatin1();
			m_vecRegisterVal[nRegiseterValIndex].u_chars[ncharIndex] = nchar;
			nCurChar++;
		}

		m_pWorkFlow->SetRegisterVal(addr, m_vecRegisterVal[nRegiseterValIndex].u_Int16[0]);
	}
	break;
	case RegisterDataType::eDataTypeInt16:
	{
		int16_t nVal;
		if (m_nIntStat == 0)
		{
            nVal = pItem->text().toInt() & 0xFFFF;
		}
        else if (m_nIntStat == 1)
		{
			bool bOk = false;
            nVal = (pItem->text().toInt(&bOk,16) & 0xFFFF);
		}

		m_vecRegisterVal[nRegiseterValIndex].u_Int16[ndataIndex % 4] = nVal;

		m_pWorkFlow->SetRegisterVal(addr, nVal);
	}
	break;
	case RegisterDataType::eDataTypeInt32:
	{
		int32_t nVal;
		if (m_nIntStat == 0)
		{
			nVal = pItem->text().toInt();
		}
        else if (m_nIntStat == 1)
		{
			bool bOk = false;
            nVal = pItem->text().toInt(&bOk,16);
		}

		m_vecRegisterVal[nRegiseterValIndex].u_Int32[ndataIndex % 2] = nVal;

		m_pWorkFlow->SetRegisterVal(addr, m_vecRegisterVal[nRegiseterValIndex].u_Int16[0]);
		m_pWorkFlow->SetRegisterVal(addr + 1, m_vecRegisterVal[nRegiseterValIndex].u_Int16[1]);
	}
	break;
	case RegisterDataType::eDataTypeFloat:
	{
		float nVal = pItem->text().toFloat();

		m_vecRegisterVal[nRegiseterValIndex].u_float[ndataIndex % 2] = nVal;

		m_pWorkFlow->SetRegisterVal(addr, m_vecRegisterVal[nRegiseterValIndex].u_Int16[0]);
		m_pWorkFlow->SetRegisterVal(addr + 1, m_vecRegisterVal[nRegiseterValIndex].u_Int16[1]);
	}
	break;
	case RegisterDataType::eDataTypeDouble:
	{
		double nVal = pItem->text().toDouble();

		m_vecRegisterVal[nRegiseterValIndex].u_double = nVal;

		m_pWorkFlow->SetRegisterVal(addr, m_vecRegisterVal[nRegiseterValIndex].u_Int16[0]);
		m_pWorkFlow->SetRegisterVal(addr + 1, m_vecRegisterVal[nRegiseterValIndex].u_Int16[1]);
		m_pWorkFlow->SetRegisterVal(addr + 2, m_vecRegisterVal[nRegiseterValIndex].u_Int16[2]);
		m_pWorkFlow->SetRegisterVal(addr + 3, m_vecRegisterVal[nRegiseterValIndex].u_Int16[3]);
	}
	break;
	default:
	break;
	}


}

bool CommTest_Qt::CheckInput(QTableWidgetItem* pItem)
{
	if (pItem == nullptr) return false;

	//判断ui->cmbBox_DataType选择类型
	int nCurIndex = ui->cmbBox_DataType->currentIndex();
	if (nCurIndex < 0) return false;
	QVariant data = ui->cmbBox_DataType->itemData(nCurIndex);

	QString text = pItem->text().trimmed();


	//跳过偶数列的单元格输入判定
	if (pItem->column() % 2 == 0) return true;

	//根据data类型判断
	RegisterDataType type = data.value<RegisterDataType>();

	 QSignalBlocker blocker(ui->table_RegisterData->itemDelegate());

	//判断当前是否选择了16进制显示
	if (m_nIntStat == 1 && (type == RegisterDataType::eDataTypeInt16 ||
		type == RegisterDataType::eDataTypeInt32))
	{
        return CheckInput_int_Hex(pItem, type);
	}

	switch (type)
	{
	case RegisterDataType::eDataTypeChar8:
		return CheckInput_str(pItem, text);
	break;
	case RegisterDataType::eDataTypeInt16:
		return CheckInput_int(pItem, text, INT16_MIN, INT16_MAX);
	break;
	case RegisterDataType::eDataTypeInt32:
		return CheckInput_int(pItem, text, INT32_MIN, INT32_MAX);
	break;
	case RegisterDataType::eDataTypeFloat:
		return CheckInput_float(pItem, text, -FLT_MAX, FLT_MAX);
	break;
	case RegisterDataType::eDataTypeDouble:
		return CheckInput_float(pItem, text, -DBL_MAX, DBL_MAX);
	break;
	default: return false;
	}

	return true;

}


bool CommTest_Qt::CheckInput_str(QTableWidgetItem* pItem, const QString& text)
{
	//校验输入的字符长度是否小于2,若超过两个则截断,否则不处理
	if (pItem->text().length() > 2)
	{
		QMessageBox::warning(
			this,
			"输入截断",
			QString("输入值 %1 长度超过2,只保留前2位!")
			.arg(text)
			.arg(INT16_MIN)
			.arg(INT16_MAX)
		);
		pItem->setText(pItem->text().left(2));
	}

	return true;
}

bool CommTest_Qt::CheckInput_int(QTableWidgetItem* pItem, const QString& text, int32_t MinVal, int32_t MaxVal)
{

	QRegularExpression regExp("^(0|-?[1-9]\\d*)$");	//负数、正数、0
	QRegularExpressionMatch match = regExp.match(text);

	if (!match.hasMatch())
	{
		QMessageBox::warning(
			this,
			"输入非法",
			QString("输入值 %1 非整型数")
			.arg(text)
		);
		pItem->setText("0");
	}
	else
	{
		//验证后先判断数据是否在int16_t范围内
		int tmp = text.toInt();
		if (tmp > MaxVal || tmp < MinVal)
		{
			QMessageBox::warning(
				this,
				"输入重置",
				QString("输入值 %1 长度超过整数 范围(%2-%3),重置为0")
				.arg(text)
				.arg(MinVal)
				.arg(MaxVal)
			);
			pItem->setText("0");

			return true;
		}

		// QRegularExpression regExp("^(-?[0]\\d+)$");	//前导0的整数
		// QRegularExpressionMatch match = regExp.match(text);
		//如果带前导0再次处理
		//if (match.hasMatch())
		{
			pItem->setText(QString("%1").arg(tmp));
		}
	}

	return true;
}

bool CommTest_Qt::CheckInput_float(QTableWidgetItem* pItem, const QString& text, double MinVal, double MaxVal)
{
	//校验输入的数字是否符合double格式
	//QRegularExpression regExp("^(-?[0-9]\\d+|[0-9]\\d*)\\.?([0-9]\\d*)$");
	QRegularExpression regExp("^(?!-0(\\.0*)?$)-?\\d+(\\.\\d+)?$");
	QRegularExpressionMatch match = regExp.match(text);
	if (!match.hasMatch())
	{
		QMessageBox::warning(
			this,
			"输入非法",
			QString("输入值 %1 非浮点数")
			.arg(text)
		);
		pItem->setText("0.0");
	}
	else
	{
		//验证后先判断数据是否在double范围内
		double tmp = text.toDouble();
		if (tmp > MaxVal || tmp < MinVal)
		{
			QMessageBox::warning(
				this,
				"输入重置",
				QString("输入值 %1 长度超过浮点数范围(%2-%3),重置为0")
				.arg(text)
				.arg(MinVal)
				.arg(MaxVal)
			);
			pItem->setText("0.0");
		}

		//二次校验是否带前导0
		// QRegularExpression regExp("^(-?[0]\\d+)\\.?([0-9]\\d*)$");
		// QRegularExpressionMatch match = regExp.match(text);
		// if (match.hasMatch())
		{
			pItem->setText(QString("%1").arg(tmp,0,'f',6));
		}
	}

	return true;
}

bool CommTest_Qt::CheckInput_int_Hex(QTableWidgetItem* pItem, const RegisterDataType& type)
{
	if (!pItem) return false;

	int nMaxDigits = 0;
	//符合十六进制数再根据类型判断数据长度
	if (type == RegisterDataType::eDataTypeInt16)
	{
		nMaxDigits = 4;
	}
	else if (type == RegisterDataType::eDataTypeInt32)
	{
		nMaxDigits = 8;
	}

	QString text = pItem->text().trimmed();

	//校验是否符合十六进制数
	QRegularExpression hexRegExp("^[0-9A-Fa-f]*$");
	if (!hexRegExp.match(text).hasMatch())
	{
		QMessageBox::warning(
			this,
			"输入非法",
			QString("输入值 %1 非十六进制数")
			.arg(text)
		);

		QString tmp = "0";
		pItem->setText(tmp.rightJustified(nMaxDigits,'0'));

		return true;
	}

	if (text.length() > nMaxDigits)
	{
        QMessageBox::warning(
            this,
            "输入截断",
            QString("输入值 %1 超过范围,将截断输入数据!")
            .arg(text)
            .arg(nMaxDigits)
            .arg(nMaxDigits)
        );

        pItem->setText(text.left(nMaxDigits).toUpper());

		return true;
	}

	pItem->setText(text.toUpper().rightJustified(nMaxDigits,'0'));

	return true;
}

void CommTest_Qt::DisplayRegisterVals()
{
	//判断ui->cmbBox_DataType选择类型
	int nCurIndex = ui->cmbBox_DataType->currentIndex();
	if (nCurIndex < 0) return ;


	ui->table_RegisterData->blockSignals(true);

	QVariant data = ui->cmbBox_DataType->itemData(nCurIndex);

	const int rowCount = ui->table_RegisterData->rowCount();    // 行数
	const int colCount = ui->table_RegisterData->columnCount(); // 列数
	
	//遍历表格的奇数列,先将数据列清空
	QTableWidgetItem* item = nullptr;
    for (int col = 1; col < colCount; col += 2) //遍历奇数列
    {
        for (int row = 0; row < rowCount; row++)
        {
			item = ui->table_RegisterData->item(row, col);
			if (item)
			{
				item->setText("");
				item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 设为只读
			}
        }
    }

	ui->table_RegisterData->blockSignals(false);

	int ndataIndex = -1;
	int nRegiseterValIndex = -1;
	RegisterDataType type = data.value<RegisterDataType>();
	switch (type)
    {
        case RegisterDataType::eDataTypeChar8:
			DisplayRegisterVals_Char8();
		break;
		case RegisterDataType::eDataTypeInt16:
            DisplayRegisterVals_Int16();
		break;
		case RegisterDataType::eDataTypeInt32:
            DisplayRegisterVals_Int32();
		break;
		case RegisterDataType::eDataTypeFloat:
            DisplayRegisterVals_Float();
		break;
		case RegisterDataType::eDataTypeDouble:
            DisplayRegisterVals_Double();
		break;
    }
}

void CommTest_Qt::DisplayRegisterVals_Char8()
{
	const int rowCount = ui->table_RegisterData->rowCount();
	const int colCount = ui->table_RegisterData->columnCount();

	int nRegisterCount = 0; //m_vecRegisterVal计数
	int nRealDataCount = 0; //实际数据计数,根据数据类型不同而不同8,4,2,2,1
	int nCellCount = 0;
	DataTypeConvert* currentData = nullptr;

	currentData = &m_vecRegisterVal[nRegisterCount];

	for (int col = 1; col < colCount; col += 2)
	{
		for (int row = 0; row < rowCount; row++)
		{
			QString strInfo(QString("%1%2")
				.arg(QChar(currentData->u_chars[nRealDataCount++]))
				.arg(QChar(currentData->u_chars[nRealDataCount++])));

			ui->table_RegisterData->item(row, col)->setText(strInfo);
			ui->table_RegisterData->item(row, col)->setFlags(ui->table_RegisterData->item(row, col)->flags() | Qt::ItemIsEditable); // 设为可编辑
			
			if (nRealDataCount == 8)
			{
				nRealDataCount = 0;
				nRegisterCount++;

				if (nRegisterCount >= m_vecRegisterVal.size())
				{
					break;
				}

				currentData = &m_vecRegisterVal[nRegisterCount];
			}
		}
	}
}

void CommTest_Qt::DisplayRegisterVals_Int16()
{
	const int rowCount = ui->table_RegisterData->rowCount();
	const int colCount = ui->table_RegisterData->columnCount();

	int nRegisterCount = 0; //m_vecRegisterVal计数
	int nRealDataCount = 0; //实际数据计数,根据数据类型不同而不同8,4,2,2,1
	int nCellCount = 0;
	DataTypeConvert* currentData = nullptr;

	currentData = &m_vecRegisterVal[nRegisterCount];

	for (int col = 1; col < colCount; col += 2)
	{
		for (int row = 0; row < rowCount; row++)
		{
			QString strInfo;
			if (m_nIntStat == 1)
			{
				strInfo = QString("%1").arg(QString::number(currentData->u_Int16[nRealDataCount++], 16), 4, '0').toUpper();
			}
			else if (m_nIntStat == 0)
			{
				strInfo = QString("%1").arg(currentData->u_Int16[nRealDataCount++]);
			}

			ui->table_RegisterData->item(row, col)->setText(strInfo);
			ui->table_RegisterData->item(row, col)->setFlags(ui->table_RegisterData->item(row, col)->flags() | Qt::ItemIsEditable); // 设为可编辑
			
			if (nRealDataCount == 4)
			{
				nRealDataCount = 0;
				nRegisterCount++;
				if (nRegisterCount >= m_vecRegisterVal.size())
				{
					break;
				}
				currentData = &m_vecRegisterVal[nRegisterCount];
			}
		}
	}
}

void CommTest_Qt::DisplayRegisterVals_Int32()
{
	const int rowCount = ui->table_RegisterData->rowCount();
	const int colCount = ui->table_RegisterData->columnCount();

	int nRegisterCount = 0; //m_vecRegisterVal计数
	int nRealDataCount = 0; //实际数据计数,根据数据类型不同而不同8,4,2,2,1
	int nCellCount = 0;
	DataTypeConvert* currentData = nullptr;

	currentData = &m_vecRegisterVal[nRegisterCount];

	for (int col = 1; col < colCount; col += 2)
	{
		for (int row = 0; row < rowCount; row++)
		{
			nCellCount++;

			if (nCellCount % 2 == 1)
			{
				QString strInfo;

				if (m_nIntStat == 1)
				{
					strInfo = QString("%1").arg(QString::number(currentData->u_Int32[nRealDataCount++], 16), 8, '0').toUpper();
				}
				else if (m_nIntStat == 0)
				{
					strInfo = QString("%1").arg(currentData->u_Int32[nRealDataCount++]);
				}

				ui->table_RegisterData->item(row, col)->setText(strInfo);
				ui->table_RegisterData->item(row, col)->setFlags(ui->table_RegisterData->item(row, col)->flags() | Qt::ItemIsEditable); // 设为可编辑
				

				if (nRealDataCount == 2)
				{
					nRealDataCount = 0;
					nRegisterCount++;
					if (nRegisterCount >= m_vecRegisterVal.size())
					{
						break;
					}
					currentData = &m_vecRegisterVal[nRegisterCount];
				}
			}
		}
	}
}

void CommTest_Qt::DisplayRegisterVals_Float()
{
	const int rowCount = ui->table_RegisterData->rowCount();
	const int colCount = ui->table_RegisterData->columnCount();

	int nRegisterCount = 0; //m_vecRegisterVal计数
	int nRealDataCount = 0; //实际数据计数,根据数据类型不同而不同8,4,2,2,1
	int nCellCount = 0;
	DataTypeConvert* currentData = nullptr;

	currentData = &m_vecRegisterVal[nRegisterCount];

	for (int col = 1; col < colCount; col += 2)
	{
		for (int row = 0; row < rowCount; row++)
		{
			nCellCount++;

			if (nCellCount % 2 == 1)
			{

				QString strInfo = QString("%1")
					.arg(currentData->u_float[nRealDataCount++]);

				ui->table_RegisterData->item(row, col)->setText(strInfo);
				ui->table_RegisterData->item(row, col)->setFlags(ui->table_RegisterData->item(row, col)->flags() | Qt::ItemIsEditable); // 设为可编辑
				
				if (nRealDataCount == 2)
				{
					nRealDataCount = 0;
					nRegisterCount++;
					if (nRegisterCount >= m_vecRegisterVal.size())
					{
						break;
					}
					currentData = &m_vecRegisterVal[nRegisterCount];
				}
			}
		}
	}
}

void CommTest_Qt::DisplayRegisterVals_Double()
{
	const int rowCount = ui->table_RegisterData->rowCount();
	const int colCount = ui->table_RegisterData->columnCount();

	int nRegisterCount = 0; //m_vecRegisterVal计数
	int nRealDataCount = 0; //实际数据计数,根据数据类型不同而不同8,4,2,2,1
	int nCellCount = 0;
	DataTypeConvert* currentData = nullptr;

	currentData = &m_vecRegisterVal[nRegisterCount];
	for (int col = 1; col < colCount; col += 2)
	{
		for (int row = 0; row < rowCount; row++)
		{
			nCellCount++;

			if (nCellCount % 4 == 1)
			{
				QString strInfo = QString("%1")
					.arg(currentData->u_double);

				ui->table_RegisterData->item(row, col)->setText(strInfo);
				ui->table_RegisterData->item(row, col)->setFlags(ui->table_RegisterData->item(row, col)->flags() | Qt::ItemIsEditable); // 设为可编辑

				nRegisterCount++;
				if (nRegisterCount >= m_vecRegisterVal.size())
				{
					break;
				}
				currentData = &m_vecRegisterVal[nRegisterCount];
			}
		}
	}
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

void CommTest_Qt::OnMovePlatformAbsInt32(int32_t x, int32_t y, int32_t angle)
{
	if (m_simulationPlatform == nullptr)
	{
		return;
	}

	// 获取除数：10的幂次方
	double divisorXY = GetDivisorFromPowerEdit(ui->edit_Unit_XY);
	double divisorD = GetDivisorFromPowerEdit(ui->edit_Unit_D);

	// 转换坐标值：除以10的幂次方
	double convertedX = static_cast<double>(x) / divisorXY;
	double convertedY = static_cast<double>(y) / divisorXY;
	double convertedAngle = static_cast<double>(angle) / divisorD;

	// 控制平台移动
	m_simulationPlatform->SetRealTimePlatformAbs(convertedX, convertedY, convertedAngle);
}

void CommTest_Qt::OnMovePlatformAbsFloat(double x, double y, double angle)
{
	if (m_simulationPlatform == nullptr)
	{
		return;
	}

	// Float类型直接使用，无需转换
	m_simulationPlatform->SetRealTimePlatformAbs(x, y, angle);
}

void CommTest_Qt::OnMovePlatformRelativeInt32(int32_t x, int32_t y, int32_t angle)
{
	if (m_simulationPlatform == nullptr)
	{
		return;
	}

	// 获取除数：10的幂次方
	double divisorXY = GetDivisorFromPowerEdit(ui->edit_Unit_XY);
	double divisorD = GetDivisorFromPowerEdit(ui->edit_Unit_D);

	// 转换坐标值：除以10的幂次方
	double convertedX = static_cast<double>(x) / divisorXY;
	double convertedY = static_cast<double>(y) / divisorXY;
	double convertedAngle = static_cast<double>(angle) / divisorD;

	// 控制平台移动
	m_simulationPlatform->SetRealTimePlatformRelative(convertedX, convertedY, convertedAngle);
}

void CommTest_Qt::OnMovePlatformRelativeFloat(double x, double y, double angle)
{
	if (m_simulationPlatform == nullptr)
	{
		return;
	}

	// Float类型直接使用，无需转换
	m_simulationPlatform->SetRealTimePlatformRelative(x, y, angle);
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
		ui->text_CommLog->append("错误: 轴位置地址无效");
		return;
	}

	// 计算需要的数组索引
	// 每个DataTypeConvert包含4个int16，可以存储2个int32
	// X占用2个int16（地址startAddr和startAddr+1）
	// Y占用2个int16（地址startAddr+2和startAddr+3）
	// Angle占用2个int16（地址startAddr+4和startAddr+5）
	if (startAddr >= REGISTER_VAL_NUM - 6)
	{
		ui->text_CommLog->append("错误: 寄存器地址超出范围");
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

	ui->text_CommLog->append(QString("轴位置双字写入成功: X=%1, Y=%2, Angle=%3 (地址:%4)")
		.arg(xInt32).arg(yInt32).arg(angleInt32).arg(startAddr));

	UpdateTableInfo(ui->edit_RegisterAddr->text().toUInt());
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
		ui->text_CommLog->append("错误: 轴位置地址无效");
		return;
	}

	if (startAddr >= REGISTER_VAL_NUM - 6)
	{
		ui->text_CommLog->append("错误: 寄存器地址超出范围");
		return;
	}

	DataTypeConvert data;
	data.u_Int32[0] = xFloat;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);
	data.u_Int32[0] = yFloat;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);
	data.u_Int32[0] = angleFloat;
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[0]);
	m_pWorkFlow->SetRegisterVal(startAddr++, data.u_Int16[1]);

	ui->text_CommLog->append(QString("轴位置浮点写入成功: X=%1, Y=%2, Angle=%3 (地址:%4)")
		.arg(xFloat).arg(yFloat).arg(angleFloat).arg(startAddr));

	UpdateTableInfo(ui->edit_RegisterAddr->text().toUInt());
}

// ====================菜单栏相关槽函数实现====================

void CommTest_Qt::OnShowAboutDialog()
{
	QMessageBox aboutBox(this);
	aboutBox.setWindowTitle("关于CommTest_Qt");
	
	// 获取编译时间
	QString compileDate = QString::fromLatin1(__DATE__);
	QString compileTime = QString::fromLatin1(__TIME__);
	
	// 设置信息内容
	QString aboutText = QString(
		"<h2>CommTest_Qt</h2>"
		"<p><b>版本：</b> Version 1.0.0</p>"
		"<p><b>编译日期：</b> %1</p>"
		"<p><b>编译时间：</b> %2</p>"
		"<p><b>作者：</b> Wang Mao</p>"
		"<hr>"
		"<p>一个基于Qt6的通信测试平台.支持Lua脚本操作</p>"
	).arg(compileDate).arg(compileTime);
	
	aboutBox.setText(aboutText);
	aboutBox.setIcon(QMessageBox::Information);
	aboutBox.setStandardButtons(QMessageBox::Ok);
	
	aboutBox.exec();
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
	//if (ui->cmbBox_ProtocolType != nullptr) ui->cmbBox_ProtocolType->setStyleSheet(inputStyleSheet);
	//if (ui->cmbBox_DataType != nullptr) ui->cmbBox_DataType->setStyleSheet(inputStyleSheet);
	
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