#include "CommTest_Qt.h"

CommTest_Qt::CommTest_Qt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CommTest_QtClass())
{
    ui->setupUi(this);
	
	m_CurInfo = nullptr;
	m_pWorkFlow = nullptr;
	m_subWindow = nullptr;
	
	m_nLogStat = 0;
	m_nIntStat = 0;
	//初始化m_vecRegisterVal
	int dataCellCount = REGISTER_TABLE_ROW_COUNT * (REGISTER_TABLE_COLUMN_COUNT / 2);
	int convertCount = (dataCellCount + 3) / 4;  // 向上取整到4的倍数

	m_vecRegisterVal.resize(convertCount);

	//初始化成员实例
	InitializeMember();
	
	//m_pWorkFlow->CreateCommProtocol(ProtocolType::eProRegMitsubishiQBinary);

	//初始化信号槽连接
	InitialSignalConnect();
	
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
    delete ui;
}

#if 0

void CommTest_Qt::InitialSocketPort()
{
#if 0
	std::unique_ptr<QTcpSocket> thistcp = std::make_unique<QTcpSocket>();

	// 连接服务器（超时 5 秒）
	thistcp->connectToHost("127.0.0.1", 20008);
	if (!thistcp->waitForConnected(5000)) {
		qDebug() << "连接失败：" << thistcp->errorString();
		return;
	}
	qDebug() << "连接成功";

	// 发送数据并确认发送完成
	QString LineString = ui->edit_ScriptName_1->text();
	QByteArray sendData = LineString.toUtf8();//"RDS DM00100.H 0002";
	thistcp->write(sendData);
	if (!thistcp->waitForBytesWritten(5000)) {
		qDebug() << "数据发送失败：" << thistcp->errorString();
		thistcp->close();
		return;
	}
	qDebug() << "数据发送成功：" << sendData;

	// 等待并接收响应
	if (!thistcp->waitForReadyRead(5000)) {
		qDebug() << "接收响应失败：" << thistcp->errorString();
		thistcp->close();
		return;
	}
	QByteArray response = thistcp->readAll();
	qDebug() << "收到服务器响应：" << response;

	// 关闭连接
	thistcp->close();
#else
	m_tcp = new QTcpServer(this);

	connect(ui->Btn_Create, &QPushButton::clicked, this, [=]() {
		if (m_tcp->isListening())
		{
			//关闭服务器并断开客户端的连接
			ui->text_CommLog->append(QString("即将断开所有客户端链接,当前链接客户端数量[%1]").arg(m_clientList.size()));
			for (QTcpSocket* socket : m_clientList)
			{
				//断开与客户端的连接
				socket->disconnectFromHost();
				if (socket->state() != QAbstractSocket::UnconnectedState) {
					socket->abort();
				}

				ui->text_CommLog->append(QString("[%1:%2] 客户端已断开")
					.arg(socket->peerAddress().toString())
					.arg(socket->peerPort()));
			}

			ui->text_CommLog->append(QString("[%1:%2] 即将关闭服务器...")
				.arg(m_tcp->serverAddress().toString())
				.arg(m_tcp->serverPort()));

			m_tcp->close();
			ui->text_CommLog->append(QString("服务器已关闭"));


			//关闭m_tcp后恢复界面状态
			ui->Btn_Create->setText("打开链接");
			ui->edit_IP->setEnabled(true);
			ui->edit_Port->setEnabled(true);
		}
		else
		{
			const QString address_text = ui->edit_IP->text();
			const QHostAddress address = QHostAddress(address_text);
			const unsigned short port = ui->edit_Port->text().toUShort();
			//开始监听，并判断是否成功
			if (m_tcp->listen(address, port)) {
				//连接成功就修改界面按钮提示，以及地址栏不可编辑

				ui->text_CommLog->append(QString("[%1:%2] 服务器创建成功,开始监听客户端链接...")
					.arg(m_tcp->serverAddress().toString())
					.arg(m_tcp->serverPort()));

				ui->Btn_Create->setText("关闭链接");
				ui->edit_IP->setEnabled(false);
				ui->edit_Port->setEnabled(false);
			}
		}

		});


	//监听到新的客户端连接请求
	connect(m_tcp, &QTcpServer::newConnection, this, [this] {
		//如果有新的连接就取出
		while (m_tcp->hasPendingConnections())
		{
			//nextPendingConnection返回下一个挂起的连接作为已连接的QTcpSocket对象
			//套接字是作为服务器的子级创建的，这意味着销毁QTcpServer对象时会自动删除该套接字。
			//最好在完成处理后显式删除该对象，以避免浪费内存。
			//返回的QTcpSocket对象不能从另一个线程使用，如有需要可重写incomingConnection().
			QTcpSocket* socket = m_tcp->nextPendingConnection();
			m_clientList.append(socket);
			ui->text_CommLog->append(QString("[%1:%2] 客户端链接成功")
				.arg(socket->peerAddress().toString())
				.arg(socket->peerPort()));

			//关联相关操作的信号槽
			//收到数据，触发readyRead
			connect(socket, &QTcpSocket::readyRead, [this, socket] {
				//没有可读的数据就返回
				if (socket->bytesAvailable() <= 0)
					return;
				//注意收发两端文本要使用对应的编解码
				const QString recv_text = QString::fromUtf8(socket->readAll());
				ui->text_CommLog->append(QString("[%1:%2]")
					.arg(socket->peerAddress().toString())
					.arg(socket->peerPort()));
				ui->text_CommLog->append(recv_text);
				});

			//error信号在5.15换了名字
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
			//错误信息
			connect(socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
				[this, socket](QAbstractSocket::SocketError) {
					ui->textRecv->append(QString("[%1:%2] 客户端错误:%3")
						.arg(socket->peerAddress().toString())
						.arg(socket->peerPort())
						.arg(socket->errorString()));
				});
#else
			//错误信息
			connect(socket, &QAbstractSocket::errorOccurred, [this, socket](QAbstractSocket::SocketError) {
				ui->text_CommLog->append(QString("[%1:%2] 客户端错误:%3")
					.arg(socket->peerAddress().toString())
					.arg(socket->peerPort())
					.arg(socket->errorString()));
				});
#endif

			//连接断开，销毁socket对象，这是为了开关server时socket正确释放
			connect(socket, &QTcpSocket::disconnected, [this, socket] {
				socket->deleteLater();
				m_clientList.removeOne(socket);
				ui->text_CommLog->append(QString("[%1:%2] 客户端断开链接")
					.arg(socket->peerAddress().toString())
					.arg(socket->peerPort()));
				//updateState();
				});
		}
		//updateState();
		});

	// 	//server向client发送内容
	// 	connect(ui->btnSend, &QPushButton::clicked, [this] {
	// 		//判断是否开启了server
	// 		if (!server->isListening())
	// 			return;
	// 		//将发送区文本发送给客户端
	// 		const QByteArray send_data = ui->textSend->toPlainText().toUtf8();
	// 		//数据为空就返回
	// 		if (send_data.isEmpty())
	// 			return;
	// 		for (QTcpSocket* socket : clientList)
	// 		{
	// 			socket->write(send_data);
	// 			//socket->waitForBytesWritten();
	// 		}
	// 		});

		//server的错误信息
		//如果发生错误，则serverError()返回错误的类型，
		//并且可以调用errorString()以获取对所发生事件的易于理解的描述
	connect(m_tcp, &QTcpServer::acceptError, [this](QAbstractSocket::SocketError) {
		ui->text_CommLog->append("服务器错误:" + m_tcp->errorString());
		});

#endif

}
#endif


void CommTest_Qt::InitializeMember()
{
// 	if (m_CurInfo == nullptr)
// 	{
// 		m_CurInfo = std::make_unique<CommBase::CommInfoBase>();
// 
// 	}

	if (m_pWorkFlow == nullptr)
	{
		m_pWorkFlow = MainWorkFlow::InitialWorkFlow(this);
	}

	// 初始化小窗口
	m_subWindow = std::make_unique<SubMainWindow>();
    
    // 初始化模拟平台窗口
    m_simulationPlatform = new SimulationPlatform();

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
	//20251024	wm	 连接小窗口的显示主窗口信号到主窗口的show()槽
	connect(m_subWindow.get(), &SubMainWindow::showMainWindow, this, &CommTest_Qt::show);

	// 连接显示/隐藏模拟平台窗口按钮
	connect(ui->Btn_ShowPlatform, &QPushButton::clicked, this, [=]() {
		if (m_simulationPlatform->isVisible()) {
			m_simulationPlatform->hide();
		} else {
			m_simulationPlatform->show();
		}
	});

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


		connect(delegate, &QAbstractItemDelegate::closeEditor, this, [this]() {
			QTableWidgetItem* currentItem = ui->table_RegisterData->currentItem();
			//判断指针有效
            if (currentItem)
            {
                UpdateRegisterVals(currentItem);
            }
		});
	}

	//20251102	wm	切换协议
	connect(ui->cmbBox_ProtocolType, &QComboBox::currentIndexChanged, this, &CommTest_Qt::CreateCurrentProtocol);

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
    connect(ui->cmbBox_DataType, &QComboBox::currentIndexChanged, this, &CommTest_Qt::DisplayRegisterVals);

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
		strLuaPath += "test.lua";
		if (!m_pWorkFlow->RunLuaScript(0,strLuaPath))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Edit_1,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		QString strLuaPath = QCoreApplication::applicationDirPath();
		strLuaPath += "/Config/LuaScript/";
		strLuaPath += "test.lua";
		
		ScriptEditor* pScriptEditor = new ScriptEditor(this,m_pWorkFlow->GetLuaScript(0));
		pScriptEditor->show();
		//获取当前执行程序路径
		
	});

	connect(ui->Btn_Execute_2,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		if (!m_pWorkFlow->RunLuaScript(1,"D:/Qt/Qt_Project/CommTest_Qt/LuaScript/LuaScript.lua"))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Execute_3,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		if (!m_pWorkFlow->RunLuaScript(2,"D:/Qt/Qt_Project/CommTest_Qt/LuaScript/LuaScript.lua"))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Execute_4,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		if (!m_pWorkFlow->RunLuaScript(3,"D:/Qt/Qt_Project/CommTest_Qt/LuaScript/LuaScript.lua"))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Execute_5,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		if (!m_pWorkFlow->RunLuaScript(4,"D:/Qt/Qt_Project/CommTest_Qt/LuaScript/LuaScript.lua"))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});

	connect(ui->Btn_Execute_6,&QPushButton::clicked, this, [=] {
		if (m_pWorkFlow == nullptr)	return;

		if (!m_pWorkFlow->RunLuaScript(5,"D:/Qt/Qt_Project/CommTest_Qt/LuaScript/LuaScript.lua"))
		{
			ui->text_CommLog->append("执行Lua脚本失败!");
		}
	});
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

// 	ui->table_RegisterData->setStyleSheet(
// 		"QTableWidget {"
// 		"   background-color: rgb(255, 255, 255); /* 默认行背景色 */"
// 		"   alternate-background-color: rgb(240, 240, 240); /* 交替行背景色 */"
// 		"   gridline-color: rgb(200, 200, 200); /* 网格线颜色 */"
// 		"}"
// 		// 单元格未选中状态：继承行的背景色（无需额外设置，保持默认）
// 		"QTableWidget::item {"
// 		"   color: black; /* 文字色（默认） */"
// 		"   border: none; /* 去掉单元格默认边框，避免与网格线冲突 */"
// 		"}"
// 		// 单元格选中状态：强制与未选中状态一致（核心）
// 		"QTableWidget::item:selected {"
// 		"   background-color: transparent; /* 透明背景，显示所在行的原始背景（白色/浅灰） */"
// 		"   color: black; /* 文字色不变 */"
// 		"   outline: none; /* 去掉选中时的焦点边框（可选，更彻底隐藏选中痕迹） */"
// 		"}"
// 	);

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
		while (nCurChar < 2 && pItem->text().length() > 0)
		{
			
			//将每个字符转化为一个unicode并存储到对应的m_vecRegisterVal的uchars数组中
			int ncharIndex = ndataIndex % 8 + nCurChar;
			m_vecRegisterVal[nRegiseterValIndex].u_chars[ncharIndex] = pItem->text().at(nCurChar).toLatin1();
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
	// 空字符串直接返回true，允许清空
// 	if (text.isEmpty()) 
// 	{
// 		return true;
// 	}

	//根据data类型判断
	RegisterDataType type = data.value<RegisterDataType>();

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
// 	{
// 		QString text = pItem->text().trimmed();
// 		QRegularExpression regExp("^(-?[0-9]\\d+|[0-9]\\d*)$");	//负数、正数、0
// 		QRegularExpressionMatch match = regExp.match(text);
// 
// 		if (!match.hasMatch() && !text.isEmpty())
// 		{
// 			pItem->setText("0");
// 		}
// 		else
// 		{
// 			QRegularExpression regExp("^(-?[0]\\d+)$");	//前导0的整数
// 			QRegularExpressionMatch match = regExp.match(text);
// 
// 			//如果带前导0再次处理
// 			if (match.hasMatch())
// 			{
// 				int tmp = text.toInt();
// 
// 				//校验整数是否在int32_t范围内
// 				if (tmp > INT32_MAX || tmp < INT32_MIN)
// 				{
// 					pItem->setText("0");
// 				}
// 				else
// 				{
// 					pItem->setText(QString("%1").arg(tmp));
// 				}
// 			}
// 		}
// 	}
	break;
	case RegisterDataType::eDataTypeFloat:
		return CheckInput_float(pItem, text, -FLT_MAX, FLT_MAX);
// 	{
// 		//校验输入的字符串是否符合浮点数格式
// 		QString text = pItem->text().trimmed();
// 		QRegularExpression regExp("^(-?[0-9]\\d+|[0-9]\\d*)\\.?([0-9]\\d*)$");
// 		QRegularExpressionMatch match = regExp.match(text);
// 		if (!match.hasMatch() && !text.isEmpty())
// 		{
// 			pItem->setText("0.0");
// 		}
// 		else
// 		{
// 			//二次校验是否带前导0
// 			QRegularExpression regExp("^(-?[0]\\d+)\\.?([0-9]\\d*)$");
// 			QRegularExpressionMatch match = regExp.match(text);
// 			if (match.hasMatch())
// 			{
// 				float tmp = text.toFloat();
// 				if (tmp > FLT_MAX || tmp < -FLT_MAX)
// 					pItem->setText("0.0");
// 				else
// 				{
// 					pItem->setText(QString("%1").arg(tmp));
// 				}
// 			}
// 
// 		}
// 	}
	break;
	case RegisterDataType::eDataTypeDouble:
		return CheckInput_float(pItem, text, -DBL_MAX, DBL_MAX);
// 	{
// 		//校验输入的数字是否符合double格式
// 		QString text = pItem->text().trimmed();
// 		QRegularExpression regExp("^(-?[0-9]\\d+|[0-9]\\d*)\\.?([0-9]\\d*)$");
// 		QRegularExpressionMatch match = regExp.match(text);
// 		if (!match.hasMatch() && !text.isEmpty())
// 		{
// 			pItem->setText("0.0");
// 		}
// 		else
// 		{
// 			//二次校验是否带前导0
// 			QRegularExpression regExp("^(-?[0]\\d+)\\.?([0-9]\\d*)$");
// 			QRegularExpressionMatch match = regExp.match(text);
// 			if (match.hasMatch())
// 			{
// 				double tmp = text.toDouble();
// 				if (tmp > DBL_MAX || tmp < -DBL_MAX)
// 					pItem->setText("0.0");
// 				else
// 				{
// 					pItem->setText(QString("%1").arg(tmp));
// 				}
// 			}
// 		}
// 	}
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

	//禁用寄存器表格修改信号
    ui->table_RegisterData->blockSignals(true);

	QVariant data = ui->cmbBox_DataType->itemData(nCurIndex);

	const int rowCount = ui->table_RegisterData->rowCount();    // 行数
	const int colCount = ui->table_RegisterData->columnCount(); // 列数
	
	//遍历表格的奇数列,先将数据列清空
    for (int col = 1; col < colCount; col += 2) //遍历奇数列
    {
        for (int row = 0; row < rowCount; row++)
        {
            ui->table_RegisterData->item(row, col)->setText("");
        }
    }


	int ndataIndex = -1;
	int nRegiseterValIndex = -1;
	RegisterDataType type = data.value<RegisterDataType>();
	switch (type)
    {
        case RegisterDataType::eDataTypeChar8:
			DisplayRegisterVals_Char8();
// 		{
//  			for (int col = 1; col < colCount; col += 2) //遍历奇数列
//  			{
//  				int group = col / 2;
//  				int nIndex = 0;
//  				for (int row = 0; row < rowCount; row++)
//  				{
//  					ndataIndex = row + group * rowCount;
//  					nRegiseterValIndex = ndataIndex / 4;
//  
//  					//QString strInfo = QString::fromLatin1(reinterpret_cast<const char*>(m_vecRegisterVal[nRegiseterValIndex].u_chars), 2);
//  					QString strInfo = QString("%1%2")
//  						.arg(QChar(m_vecRegisterVal[nRegiseterValIndex].u_chars[nIndex++]))
//                          .arg(QChar(m_vecRegisterVal[nRegiseterValIndex].u_chars[nIndex++]));
//  					if (nIndex > 7)
//  					{
//  						nIndex = 0;
//  					}
//  
//  					ui->table_RegisterData->item(row, col)->setText(strInfo);
//  				}
//  			}
// 		}
		break;
		case RegisterDataType::eDataTypeInt16:
            DisplayRegisterVals_Int16();
// 		{
// 			for (int col = 1; col < colCount; col += 2) //遍历奇数列
// 			{
// 				int group = col / 2;
// 				int nIndex = 0;
// 				for (int row = 0; row < rowCount; row++)
// 				{
// 					ndataIndex = row + group * rowCount;
// 					nRegiseterValIndex = ndataIndex / 4;
// 
// 					QString strInfo = QString("%1").arg(m_vecRegisterVal[nRegiseterValIndex].u_Int16[nIndex++]);
// 					if (nIndex > 3)
// 					{
// 						nIndex = 0;
// 					}
// 					ui->table_RegisterData->item(row, col)->setText(strInfo);
// 				}
// 			}
// 		}
		break;
		case RegisterDataType::eDataTypeInt32:
            DisplayRegisterVals_Int32();
// 		{
// 			for (int col = 1; col < colCount; col += 2) //遍历奇数列
// 			{
// 				int group = col / 2;
// 				int nIndex = 0;
// 				for (int row = 0; row < rowCount; row += 2)
// 				{
// 					ndataIndex = row + group * rowCount;
// 					nRegiseterValIndex = ndataIndex / 4;
// 
// 					QString strInfo = QString("%1").arg(m_vecRegisterVal[nRegiseterValIndex].u_Int32[nIndex++]);
// 					if (nIndex > 1)
// 					{
// 						nIndex = 0;
// 					}
// 					ui->table_RegisterData->item(row, col)->setText(strInfo);
// 				}
// 			}
// 		}
		break;
		case RegisterDataType::eDataTypeFloat:
            DisplayRegisterVals_Float();
// 		{
// 			for (int col = 1; col < colCount; col += 2) //遍历奇数列
// 			{
// 				int group = col / 2;
// 				int nIndex = 0;
// 				for (int row = 0; row < rowCount; row += 2)
// 				{
// 					ndataIndex = row + group * rowCount;
// 					nRegiseterValIndex = ndataIndex / 4;
// 
// 					QString strInfo = QString("%1").arg(m_vecRegisterVal[nRegiseterValIndex].u_float[nIndex++]);
// 					if (nIndex > 1)
// 					{
// 						nIndex = 0;
// 					}
// 					ui->table_RegisterData->item(row, col)->setText(strInfo);
// 				}
// 			}
// 		}
		break;
		case RegisterDataType::eDataTypeDouble:
            DisplayRegisterVals_Double();
// 		{
// 			for (int col = 1; col < colCount; col += 2) //遍历奇数列
// 			{
// 				int group = col / 2;
// 
// 				for (int row = 0; row < rowCount; row += 4)
// 				{
// 					ndataIndex = row + group * rowCount;
// 					nRegiseterValIndex = ndataIndex / 4;
// 
// 					QString strInfo = QString("%1").arg(m_vecRegisterVal[nRegiseterValIndex].u_double);
// 
// 					ui->table_RegisterData->item(row, col)->setText(strInfo);
// 				}
// 			}
// 		}
		break;
    }

    ui->table_RegisterData->blockSignals(false);
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

// 	for (int col = 1; col < colCount; col += 2)
// 	{
// 		int group = col / 2;
// 		int nIndex = 0;
// 		int currentRegisterIndex = -1;
// 		DataTypeConvert* currentData = nullptr;
// 
// 		for (int row = 0; row < rowCount; row += 2)
// 		{
// 			int ndataIndex = row + group * rowCount;
// 			int nRegisterValIndex = ndataIndex / 4;
// 
// 			// 只在需要时更新当前数据指针
// 			if (nRegisterValIndex != currentRegisterIndex)
// 			{
// 				if (nRegisterValIndex < m_vecRegisterVal.size())
// 				{
// 					currentData = &m_vecRegisterVal[nRegisterValIndex];
// 					currentRegisterIndex = nRegisterValIndex;
// 					nIndex = 0;
// 				}
// 				else
// 				{
// 					currentData = nullptr;
// 				}
// 			}
// 
// 			if (currentData && nIndex < 2)
// 			{
// 				QString strInfo = QString("%1")
// 					.arg(currentData->u_float[nIndex++]);
// 
// 				ui->table_RegisterData->item(row, col)->setText(strInfo);
// 			}
// 			else
// 			{
// 				ui->table_RegisterData->item(row, col)->setText("");
// 			}
// 		}
// 	}
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

				nRegisterCount++;
				if (nRegisterCount >= m_vecRegisterVal.size())
				{
					break;
				}
				currentData = &m_vecRegisterVal[nRegisterCount];
			}
		}
	}

// 	for (int col = 1; col < colCount; col += 2)
// 	{
// 		int group = col / 2;
// 		int nIndex = 0;
// 		int currentRegisterIndex = -1;
// 		DataTypeConvert* currentData = nullptr;
// 
// 		for (int row = 0; row < rowCount; row += 4)
// 		{
// 			int ndataIndex = row + group * rowCount;
// 			int nRegisterValIndex = ndataIndex / 4;
// 
// 			// 只在需要时更新当前数据指针
// 			if (nRegisterValIndex != currentRegisterIndex)
// 			{
// 				if (nRegisterValIndex < m_vecRegisterVal.size())
// 				{
// 					currentData = &m_vecRegisterVal[nRegisterValIndex];
// 					currentRegisterIndex = nRegisterValIndex;
// 					nIndex = 0;
// 				}
// 				else
// 				{
// 					currentData = nullptr;
// 				}
// 			}
// 
// 			if (currentData)
// 			{
// 				QString strInfo = QString("%1")
// 					.arg(currentData->u_double);
// 
// 				ui->table_RegisterData->item(row, col)->setText(strInfo);
// 			}
// 			else
// 			{
// 				ui->table_RegisterData->item(row, col)->setText("");
// 			}
// 		}
// 	}
}
