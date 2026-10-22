/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "MainWindow.h"
#include "AuxDialogs.h"
#include "SimulationPlatform/SimulationPlatform.h"
#include "Theme/ThemeManager.h"
#include "I18n/LanguageManager.h"
#include "Core/RegisterStore.h"
#include "Comm/Socket/CommSocket.h"
#include "Comm/CommInfoFactory.h"
#include "PlatformBinding.h"
#include "version.h"
#include <QWindow>
#include <QScreen>
#include <QVariantMap>
#include <QMap>
#include <QButtonGroup>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QIntValidator>
#include <QTextDocument>
#include <QSignalBlocker>
#include <QImageReader>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QIcon>
#include <QSize>
#include <QCoreApplication>
#include <QApplication>

namespace
{
// 寄存器表维度(地址/值成对,列须偶数)。原为 RegisterTableController.h 内的宏,本处统一持有。
constexpr int kRegRows = 21;
constexpr int kRegCols = 10;

// 通信日志最大行数,超出即整体清空
constexpr int kMaxLogLines = 5000;

// 单个平台位姿占用的寄存器数(X/Y/Z 三段,每段 kAxisFieldStride 个)
constexpr int kAxisRegSpan = 6;
// 位姿字段步长(每轴占 2 个寄存器,X 起址、Y=起址+步长、Z=起址+2*步长)
constexpr int kAxisFieldStride = 2;

// 单色 SVG 图标按目标尺寸矢量栅格化,再整体染成主题前景色返回 QIcon。
// 走 QImageReader + qsvg 运行期插件(无需链接 Qt Svg 模块);SourceIn 以图形 alpha 为遮罩重填颜色。
QIcon makeTintedIcon(const QString& resPath, int px, const QColor& color, qreal dpr)
{
	QImageReader reader(resPath);
	reader.setScaledSize(QSize(qRound(px * dpr), qRound(px * dpr)));  // 按设备像素栅格化,HiDPI 下仍锐利
	QImage img = reader.read();
	if (img.isNull())
		return QIcon();

	img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	QPainter p(&img);
	p.setCompositionMode(QPainter::CompositionMode_SourceIn);  // 保留原图 alpha,颜色替换为主题色
	p.fillRect(img.rect(), color);
	p.end();

	QPixmap pm = QPixmap::fromImage(img);
	pm.setDevicePixelRatio(dpr);
	return QIcon(pm);
}
}

void MainWindow::refreshAxisAddrStatus()
{
	// 轴写入地址并入平台段悬停详情,随平台参数变更推入状态栏控制器
	if (m_statusBarController)
		m_statusBarController->setPlatformParams(m_platformParams);
}

void MainWindow::applyPlatformParams()
{
	// 把 m_platformParams 应用到控制器(单位幂)与状态栏(供加载/编辑复用)
	if (m_platformController)
		m_platformController->setUnitPowers(m_platformParams.unitXY, m_platformParams.unitD);
	refreshAxisAddrStatus();
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow()), m_pWorkflow(nullptr), m_simulationPlatform(nullptr), m_configStore(nullptr)
{
	ui->setupUi(this);
	setWindowTitle(QString("%1 - v%2").arg(QCoreApplication::translate("AppInfo", APP_NAME)).arg(APP_VERSION));

	// 启动尺寸:取当前屏可用区的 80%,且不超过 .ui 设计尺寸(1350x971),再居中。
	// 固定像素尺寸在低逻辑分辨率显示器上会溢出屏幕(高度被任务栏/标题栏切掉),
	// 故按屏比例自适应;上限钳到设计尺寸,避免在高分屏上被撑得过大、布局发散。
	if (QScreen* scr = screen())
	{
		constexpr double kScreenFraction = 0.8;
		const QRect avail = scr->availableGeometry();
		const int w = qMin(width(),  qRound(avail.width()  * kScreenFraction));
		const int h = qMin(height(), qRound(avail.height() * kScreenFraction));
		resize(w, h);
		move(avail.x() + (avail.width()  - w) / 2,
		     avail.y() + (avail.height() - h) / 2);
	}

	createMembers();          // ① 创建所有成员对象(按依赖顺序)
	setupUiContent();         // ② 填充静态 UI(下拉框 / 只读 / 状态栏初值)
	loadConfigs();            // ③ 读取持久化配置(须先于连接信号,避免回写回环)
	setupInputValidators();   // ④ 输入校验(须晚于 loadConfigs:IP 掩码会重排已填文本)
	applyThemePref();         // ⑤ 应用持久化主题(须先于建菜单:菜单勾选读当前主题)
	connectSignals();         // ⑥ 连接所有信号槽
	initialRefresh();         // ⑦ 首屏刷新表格
}

MainWindow::~MainWindow()
{
	MainWorkflow::ReleaseWorkflow();

	delete ui;
}

void MainWindow::loadConfigs()
{
	// 脚本默认名随启动语言;有持久化名称时下方覆盖。
	// .ui 中名称框/IP/端口/地址文本标为 notr:否则切换语言时 retranslateUi 会把用户值写回默认值
	{
		const QVector<QLineEdit*> nameEdits = scriptNameEdits();
		for (int i = 0; i < nameEdits.size(); ++i)
			nameEdits[i]->setText(tr("脚本%1").arg(i + 1));
	}

	// 加载之前保存的配置
	if (m_configStore)
	{
		// 加载通信信息
		{
			QVariantMap commRec;
			if (m_configStore->LoadCommInfo(commRec))
			{
				auto commInfo = CommInfoFactory::Deserialize(commRec);
				if (commInfo && commInfo->GetCommType() == CommBase::CommType::eSocket)
				{
					auto* sock = static_cast<CommSocket::SocketCommInfo*>(commInfo.get());
					ui->edit_IP->setText(sock->m_strSocketIPAddress);
					ui->edit_Port->setText(QString::number(sock->m_nSocketPort));
				}
			}
		}

		// 应用加载的配置到UI
		// 加载脚本名称
		QStringList scriptNames;
		if (m_configStore->LoadScriptNames(scriptNames))
		{
			auto nameEdits = scriptNameEdits();
			for (int i = 0; i < nameEdits.size() && i < scriptNames.size(); ++i)
				nameEdits[i]->setText(scriptNames[i]);
		}

		// 加载协议类型
		int protocolType = -1;
		if (m_configStore->LoadProtocolType(protocolType) && protocolType >= 0)
		{
			// 根据protocolType遍历ui->cmbBox_ProtocolType查找对应索引并设置
			for (int i = 0; i < ui->cmbBox_ProtocolType->count(); ++i)
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
		QVariantMap platformParams;
		if (m_configStore->LoadSimulationPlatformParams(platformParams) && m_simulationPlatform != nullptr)
		{
			m_simulationPlatform->setSceneParamsFromMap(platformParams);
		}

		// 加载平台/轴写入参数(单位幂 + 对象/目标轴地址);须重新 apply,
		// 因 createMembers 已用默认值构造过 PlatformController
		QVariantMap axisParams;
		if (m_configStore->LoadAxisWriteParams(axisParams))
		{
			m_platformParams.fromVariantMap(axisParams);
			applyPlatformParams();
		}
	}
}

void MainWindow::createMembers()
{
	m_configStore = new ConfigStore(this);

	if (m_pWorkflow == nullptr)
	{
		m_pWorkflow = MainWorkflow::InitialWorkflow(this);
	}

	// 小窗口:标题栏保留标题与最小化按钮(支持任务栏最小化/还原),不显示最大化/关闭按钮
	m_subWindow = std::make_unique<QuickPanel>();
	m_subWindow->setWindowTitle(this->windowTitle() + tr(" - 子窗口"));
	m_subWindow->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);

	// 模拟平台窗口
	m_simulationPlatform = new SimulationPlatform(this);
	m_simulationPlatform->setWindowTitle(this->windowTitle() + tr(" - 模拟平台"));
	m_simulationPlatform->setWindowFlags(
		Qt::Dialog | Qt::WindowMinimizeButtonHint // 显示最小化按钮
		| Qt::WindowMaximizeButtonHint			  // 显示最大化按钮
		| Qt::WindowCloseButtonHint				  // 显示关闭按钮
		| Qt::WindowSystemMenuHint				  // 保留系统菜单（支持右键最小化/最大化）
	);
	m_simulationPlatform->setAttribute(Qt::WA_ShowWithoutActivating, true);

	// 寄存器表格管理器
	m_registerTableController = std::make_unique<RegisterTableController>(
		ui->table_RegisterData,
		m_pWorkflow->registerStore(),
		this);
	m_registerTableController->initTable(kRegRows, kRegCols);  // 内部自连 store、自装编辑委托

	// 脚本管理器
	m_scriptManager = std::make_unique<ScriptManager>(m_pWorkflow->scriptHost(), this);

	// 平台控制器
	m_platformController = std::make_unique<PlatformController>(
		m_pWorkflow ? m_pWorkflow->registerStore() : nullptr,
		m_simulationPlatform,
		m_platformParams.unitXY, m_platformParams.unitD, nullptr);

	// 状态栏控制器:5 段运行态信息(通信/客户端/健康/脚本/平台)
	m_statusBarController = std::make_unique<StatusBarController>(ui->statusBar, this);
}

void MainWindow::setupUiContent()
{
	// 协议类型下拉
	{
		QMap<ProtocolType, QString> protocolTypeMap;
		protocolTypeMap[ProtocolType::eProRegKeyencePCLink] = tr("基恩士PC-LINK上位链路协议");
		protocolTypeMap[ProtocolType::eProRegMitsubishiQBinary] = tr("三菱MC协议二进制通信");

		for (auto it = protocolTypeMap.begin(); it != protocolTypeMap.end(); ++it)
		{
			ui->cmbBox_ProtocolType->addItem(it.value(), QVariant::fromValue(it.key()));
		}

		ui->edit_IP->setMaximumWidth(120);
		ui->edit_Port->setMaximumWidth(70);

		if (ui->cmbBox_ProtocolType->currentIndex() < 0)
		{
			ui->cmbBox_ProtocolType->setCurrentIndex(0);
		}
		else
		{
			CreateCurrentProtocol();
		}
	}

	// 数据类型下拉
	{
		QMap<RegisterDataType, QString> dataTypeMap;
		dataTypeMap[RegisterDataType::eDataTypeChar8] = tr("字符");
		dataTypeMap[RegisterDataType::eDataTypeInt16] = tr("单字");
		dataTypeMap[RegisterDataType::eDataTypeInt32] = tr("双字");
		dataTypeMap[RegisterDataType::eDataTypeFloat] = tr("单精度");
		dataTypeMap[RegisterDataType::eDataTypeDouble] = tr("双精度");

		for (auto it = dataTypeMap.begin(); it != dataTypeMap.end(); ++it)
		{
			ui->cmbBox_DataType->addItem(it.value(), QVariant::fromValue(it.key()));
		}

		ui->cmbBox_DataType->setCurrentIndex(0);

		// 推入初始数据类型(RTM 不再实时读控件)
		m_registerTableController->setDataType(
			ui->cmbBox_DataType->currentData().value<RegisterDataType>());
	}

	ui->text_CommLog->setReadOnly(true);

	// 状态栏轴地址初值(label 已在 createMembers 创建)
	refreshAxisAddrStatus();
}

void MainWindow::applyThemePref()
{
	// 读取持久化主题偏好,默认深色;须先于 buildMenus(菜单勾选读当前主题)
	int themeId = static_cast<int>(Theme::Dark);
	m_configStore->LoadThemePref(themeId);
	ThemeManager::instance().applyTheme(static_cast<Theme>(themeId));
}

void MainWindow::initialRefresh()
{
	// 恢复寄存器表显示设置:信号屏蔽下设控件,避免触发 change 槽造成载入即回存/重复推送
	QVariantMap rv;
	if (m_configStore && m_configStore->LoadRegisterView(rv))
	{
		{
			QSignalBlocker blocker(ui->edit_RegisterAddr);
			ui->edit_RegisterAddr->setText(
				QString::number(rv.value("startAddr", ui->edit_RegisterAddr->text().toInt()).toInt()));
		}
		{
			const int savedType = rv.value("dataType", -1).toInt();
			for (int i = 0; i < ui->cmbBox_DataType->count(); ++i)
			{
				if (static_cast<int>(ui->cmbBox_DataType->itemData(i).value<RegisterDataType>()) == savedType)
				{
					QSignalBlocker blocker(ui->cmbBox_DataType);
					ui->cmbBox_DataType->setCurrentIndex(i);
					break;
				}
			}
		}
		{
			const bool hex = rv.value("numberBaseHex", false).toBool();
			QSignalBlocker b1(ui->Radio_Data_HEX);
			QSignalBlocker b2(ui->Radio_Data_DEC);
			ui->Radio_Data_HEX->setChecked(hex);
			ui->Radio_Data_DEC->setChecked(!hex);
		}
	}

	// 三条视图状态统一显式首推(不依赖 setChecked 副作用;载入后为载入值,否则默认值)
	m_registerTableController->setStartAddr(ui->edit_RegisterAddr->text().toInt());  // 内部自动刷新
	m_registerTableController->setDataType(ui->cmbBox_DataType->currentData().value<RegisterDataType>());
	m_registerTableController->setNumberBase(ui->Radio_Data_HEX->isChecked());

	m_uiReady = true;   // 启动完成:此后寄存器视图控件的用户变更才落盘
}

void MainWindow::saveRegisterView()
{
	if (!m_uiReady || !m_configStore) return;   // 屏蔽启动期控件初值触发
	QVariantMap m;
	m["startAddr"]     = ui->edit_RegisterAddr->text().toInt();
	m["dataType"]      = static_cast<int>(ui->cmbBox_DataType->currentData().value<RegisterDataType>());
	m["numberBaseHex"] = ui->Radio_Data_HEX->isChecked();
	m_configStore->SaveRegisterView(m);
}

void MainWindow::connectSignals()
{
	buildMenus();
	connectWindowSignals();
	connectRegisterTable();
	connectComm();
	connectLog();
	connectScript();
	connectStatusBar();

	// 平台绑定 + 位姿自动写入(workflow 实际恒非空,守卫仅为防御)
	if (m_pWorkflow)
	{
		m_pWorkflow->scriptHost()->installModule(
			std::make_unique<PlatformBinding>(m_platformController.get()));

		connect(m_simulationPlatform, &SimulationPlatform::poseChanged, this,
			[this](Platform which, const Pose& p) { OnPlatformPoseChanged(which, p); });
	}
}

void MainWindow::buildMenus()
{
	// 初始化菜单栏
	QMenu *helpMenu = ui->menuBar->addMenu(tr("帮助(&H)"));
	m_helpMenu = helpMenu;
	QAction *aboutAction = helpMenu->addAction(tr("关于(&A)"));
	m_actAbout = aboutAction;
	QAction *changelogAction = helpMenu->addAction(tr("更新日志(&U)"));
	m_actChangelog = changelogAction;
	connect(aboutAction, &QAction::triggered, this, [this]() { AuxDialogs::showAbout(this); });
	connect(changelogAction, &QAction::triggered, this, [this]() { AuxDialogs::showChangeLog(this); });

	// 视图菜单:主题切换
	QMenu* viewMenu = ui->menuBar->addMenu(tr("视图(&V)"));
	m_viewMenu = viewMenu;
	ui->menuBar->insertMenu(helpMenu->menuAction(), viewMenu);
	QMenu* themeMenu = viewMenu->addMenu(tr("主题"));
	m_themeMenu = themeMenu;
	QAction* lightThemeAction = themeMenu->addAction(tr("浅色"));
	QAction* darkThemeAction = themeMenu->addAction(tr("深色"));
	m_actLightTheme = lightThemeAction;
	m_actDarkTheme = darkThemeAction;
	lightThemeAction->setCheckable(true);
	darkThemeAction->setCheckable(true);
	QActionGroup* themeGroup = new QActionGroup(this);
	themeGroup->setExclusive(true);
	themeGroup->addAction(lightThemeAction);
	themeGroup->addAction(darkThemeAction);

	// 同步当前主题的勾选状态
	Theme cur = ThemeManager::instance().currentTheme();
	lightThemeAction->setChecked(cur == Theme::Light);
	darkThemeAction->setChecked(cur == Theme::Dark);

	connect(lightThemeAction, &QAction::triggered, this, [this]() { OnThemeSelected(Theme::Light); });
	connect(darkThemeAction, &QAction::triggered, this, [this]() { OnThemeSelected(Theme::Dark); });

	m_langMenu = viewMenu->addMenu(tr("语言"));
	m_actLangZh = m_langMenu->addAction(QStringLiteral("中文"));
	m_actLangEn = m_langMenu->addAction(QStringLiteral("English"));
	m_actLangZh->setCheckable(true);
	m_actLangEn->setCheckable(true);
	QActionGroup* langGroup = new QActionGroup(this);
	langGroup->setExclusive(true);
	langGroup->addAction(m_actLangZh);
	langGroup->addAction(m_actLangEn);
	const AppLanguage curLang = currentLanguage();
	m_actLangZh->setChecked(curLang == AppLanguage::Chinese);
	m_actLangEn->setChecked(curLang == AppLanguage::English);
	connect(m_actLangZh, &QAction::triggered, this, [this]() { OnLanguageSelected(AppLanguage::Chinese); });
	connect(m_actLangEn, &QAction::triggered, this, [this]() { OnLanguageSelected(AppLanguage::English); });

	// 平台菜单(顺序:视图 | 平台 | 帮助)
	QMenu* platformMenu = new QMenu(tr("平台(&P)"), this);
	m_platformMenu = platformMenu;
	ui->menuBar->insertMenu(helpMenu->menuAction(), platformMenu);

	m_actShowPlatform = platformMenu->addAction(tr("显示平台"));
	m_actShowPlatform->setCheckable(true);
	connect(m_actShowPlatform, &QAction::toggled, this, [this](bool on){
		m_simulationPlatform->setVisible(on);
	});

	m_actAutoWrite = platformMenu->addAction(tr("自动写入轴位置"));
	m_actAutoWrite->setCheckable(true);

	QMenu* fmtMenu = platformMenu->addMenu(tr("写入格式"));
	m_fmtMenu = fmtMenu;
	QActionGroup* fmtGroup = new QActionGroup(this);
	fmtGroup->setExclusive(true);
	m_actFmtFloat = fmtMenu->addAction(tr("浮点写入"));
	m_actFmtInt32 = fmtMenu->addAction(tr("双字写入"));
	m_actFmtFloat->setCheckable(true);
	m_actFmtInt32->setCheckable(true);
	fmtGroup->addAction(m_actFmtFloat);
	fmtGroup->addAction(m_actFmtInt32);
	m_actFmtInt32->setChecked(true);   // 默认双字(原 Radio_AxisPos_Int32->setChecked(true))

	platformMenu->addSeparator();

	QAction* actPlatformParams = platformMenu->addAction(tr("参数设置…"));
	m_actPlatformParams = actPlatformParams;
	connect(actPlatformParams, &QAction::triggered, this, [this]{
		if (AuxDialogs::editPlatformParams(this, m_platformParams)) {
			applyPlatformParams();
			if (m_configStore)
				m_configStore->SaveAxisWriteParams(m_platformParams.toVariantMap());
		}
	});

	// 手动写入工具栏:图标+文字(两个按钮共用"写入"图标,文字区分格式,完整名进 tooltip)
	QToolBar* platformToolBar = addToolBar(tr("平台操作"));
	m_platformToolBar = platformToolBar;
	platformToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	platformToolBar->setIconSize(QSize(18, 18));
	m_actManualFloat = platformToolBar->addAction(tr("浮点"));
	m_actManualInt32 = platformToolBar->addAction(tr("双字"));
	m_actManualFloat->setToolTip(tr("浮点写入"));
	m_actManualInt32->setToolTip(tr("双字写入"));
	updateToolbarIcons();   // 按当前主题染色设置图标
	connect(m_actManualFloat, &QAction::triggered, this, &MainWindow::OnWriteAxisFloat);
	connect(m_actManualInt32, &QAction::triggered, this, &MainWindow::OnWriteAxisDoubleWord);

	// 主题切换时重染工具栏图标(前景色随深浅变)
	connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
		[this]{ updateToolbarIcons(); });
}

void MainWindow::updateToolbarIcons()
{
	if (m_actManualFloat == nullptr || m_actManualInt32 == nullptr)
		return;

	// 两个手动写入按钮共用同一"写入"图标,染成当前主题前景色
	const QColor fg = ThemeManager::instance().color("@text");
	const QIcon icon = makeTintedIcon(":/icons/edit_register.svg", 18, fg, devicePixelRatioF());
	m_actManualFloat->setIcon(icon);
	m_actManualInt32->setIcon(icon);
}

void MainWindow::connectWindowSignals()
{
	// 主窗口状态变化 → 模拟平台跟随(仅主窗口显示时同步)
	connect(windowHandle(), &QWindow::windowStateChanged, this, [this](Qt::WindowState state)
			{
        if (this->isVisible()) {
            m_simulationPlatform->setWindowState(state);
        } });

	// 子窗口/平台窗口状态变化:用事件过滤器捕获(QWidget 级事件,不依赖原生句柄;逻辑见 eventFilter)
	m_subWindow->installEventFilter(this);
	m_simulationPlatform->installEventFilter(this);

	// 连接小窗口的显示主窗口信号到主窗口的show()槽
	connect(m_subWindow.get(), &QuickPanel::showMainWindow, this, &MainWindow::show);

	// 隐藏主窗口槽函数
	connect(ui->Btn_HideMainWindow, &QPushButton::clicked, this, [=]()
			{
				QStringList lineEditTexts;
				for (QLineEdit* e : scriptNameEdits()) lineEditTexts << e->text();

				// 设置小窗口6个按钮的文本
				if (m_subWindow != nullptr)
				{
					m_subWindow->setButtonTexts(lineEditTexts);

					// 隐藏主窗口，显示小窗口
					this->hide();

					// 设置小窗口为工具窗口，不会单独占用任务栏图标
					m_subWindow->show();
					m_subWindow->activateWindow();
				}
			});

	// 初始化SimulationPlatform自动保存参数
	connect(m_simulationPlatform, &SimulationPlatform::sceneParamsChanged, this, [this](double, double)
			{
		if (m_configStore)
		{
			// 字段由 PlatformScene 自描述,MainWindow 不再拼字段名
			m_configStore->SaveSimulationPlatformParams(m_simulationPlatform->sceneParamsToMap());
		} });
}

void MainWindow::connectScript()
{
	// 连接脚本执行/编辑/循环按钮(数量由 UI 推导:Btn_Execute_{i+1} 找不到即停)
	for (int i = 0; ; ++i)
	{
		auto* execBtn = findChild<QPushButton*>(QString("Btn_Execute_%1").arg(i + 1));
		if (!execBtn) break;   // 行数由 UI 推导:无执行按钮即停
		auto* editBtn = findChild<QPushButton*>(QString("Btn_Edit_%1").arg(i + 1));
		auto* loopChk = findChild<QCheckBox*>(QString("ChkBox_LoopEnable_%1").arg(i + 1));
		m_scriptManager->bindScriptRow(i, execBtn, editBtn, loopChk);
	}

	// 子窗口脚本执行:统一走 ScriptManager 单一入口(信号已 0-based)
	connect(m_subWindow.get(), &QuickPanel::executeLuaScript, this, [this](int scriptIndex)
			{ m_scriptManager->runScript(scriptIndex); });

	// 脚本相关提示(如脚本不存在)转发到通信日志
	connect(m_scriptManager.get(), &ScriptManager::logMessage, this, &MainWindow::UpdateLogDisplay);

	if (m_configStore)
	{
		auto saveScriptNames = [this]()
		{
			QStringList names;
			for (QLineEdit* e : scriptNameEdits()) names << e->text();
			m_configStore->SaveScriptNames(names);
		};

		// 用 editingFinished(失焦/回车)触发保存,避免 textChanged 每字符全量写配置
		for (QLineEdit* e : scriptNameEdits())
			connect(e, &QLineEdit::editingFinished, this, saveScriptNames);
	}
}

QVector<QLineEdit*> MainWindow::scriptNameEdits() const
{
	// 脚本名称编辑框在运行期不变,首次发现后缓存,避免每次全树 findChild 扫描。
	// 数量由 UI 推导:edit_ScriptName_{i} 找不到即停
	if (m_scriptNameEdits.isEmpty())
	{
		for (int i = 1; ; ++i)
		{
			auto* edit = findChild<QLineEdit*>(QString("edit_ScriptName_%1").arg(i));
			if (!edit) break;
			m_scriptNameEdits << edit;
		}
	}
	return m_scriptNameEdits;
}

void MainWindow::connectRegisterTable()
{
	// 点击清除寄存器:归零;经 dataChanged 闪红提示"已全清"
	connect(ui->Btn_ClearRegister, &QPushButton::clicked, this, [=]
			{
		if (m_pWorkflow == nullptr) return;
		m_pWorkflow->registerStore()->resetAll(0); });

	// 修改显示寄存器地址(推入即自动刷新)
	connect(ui->edit_RegisterAddr, &QLineEdit::textChanged, this, [=](const QString &text)
			{
				if (text == "")
					return;
				int nAddr = text.toInt();
				if (nAddr < 0)
					return;
				m_registerTableController->setStartAddr(nAddr);
				saveRegisterView();
			});

	// 修改显示寄存器数据类型(推入即自动刷新)
	connect(ui->cmbBox_DataType, &QComboBox::currentIndexChanged, this, [=]
			{
				m_registerTableController->setDataType(
					ui->cmbBox_DataType->currentData().value<RegisterDataType>());
				saveRegisterView();
			});

	// 数据显示进制切换(DEC/HEX,推入即自动刷新)
	QButtonGroup *group1 = new QButtonGroup(this);
	group1->addButton(ui->Radio_Data_DEC);
	group1->addButton(ui->Radio_Data_HEX);
	ui->Radio_Data_DEC->setChecked(true);
	connect(group1, &QButtonGroup::buttonToggled, this, [=](QAbstractButton *button, bool checked)
			{
		if (checked)
		{
			m_registerTableController->setNumberBase(button == ui->Radio_Data_HEX);
			saveRegisterView();
		}
			});
}

void MainWindow::connectComm()
{
	// 切换协议
	connect(ui->cmbBox_ProtocolType, &QComboBox::currentIndexChanged, this, [this](int index)
			{
		// 创建协议并保存协议类型
		CreateCurrentProtocol();
		if (m_configStore && index >= 0) {
			ProtocolType selectedType = ui->cmbBox_ProtocolType->currentData().value<ProtocolType>();
			m_configStore->SaveProtocolType(static_cast<int>(selectedType));
		} });

	// 点击打开/关闭连接按钮:仅分发,具体逻辑见 openConnection / closeConnection
	connect(ui->Btn_Create, &QPushButton::clicked, this, [this]
			{
		if (m_pWorkflow == nullptr) return;
		if (m_pWorkflow->IsCommOpen())
			closeConnection();
		else
			openConnection();
			});
}

void MainWindow::openConnection()
{
	auto info = std::make_unique<CommSocket::SocketCommInfo>();
	info->m_SocketType         = CommSocket::SocketType::eSTServer;
	info->m_strSocketIPAddress = ui->edit_IP->text();
	info->m_nSocketPort        = ui->edit_Port->text().toUShort();
	info->m_nSocketListenNum   = 10;

	// 非拥有视图,连接成功后落盘用;所有权随即转交工作流(对象仍由其持有,指针有效)
	CommBase::CommInfoBase* infoView = info.get();
	m_pWorkflow->SetCommInfo(std::move(info));

	// 状态栏:开连接前注入通信配置,使随后的连接状态信号已有上下文
	if (m_statusBarController)
		m_statusBarController->setCommConfig(ui->cmbBox_ProtocolType->currentText(),
			ui->edit_IP->text(), ui->edit_Port->text().toUShort(), true);

	if (!m_pWorkflow->OpenComm())
	{
		UpdateLogDisplay(tr("打开连接失败!"));
		return;
	}

	// 仅在连接成功后持久化,避免保存打不开的通信参数
	if (m_configStore)
	{
		m_configStore->SaveCommInfo(CommInfoFactory::Serialize(*infoView));
	}

	auto ExecuteRequest = [this](const QByteArray& in, QByteArray& out) {
		if (!m_pWorkflow) return false;
		return m_pWorkflow->ProcessRequest(in, out);
	};
	m_pWorkflow->SetRequestProcessor(ExecuteRequest);

	setCommControlsEnabled(false);
}

void MainWindow::closeConnection()
{
	if (!m_pWorkflow->CloseComm())
	{
		UpdateLogDisplay(tr("关闭连接失败!"));
		return;
	}
	setCommControlsEnabled(true);
}

void MainWindow::setCommControlsEnabled(bool enabled)
{
	// enabled=未连接态:通信参数可编辑、按钮显示"打开";否则连接态:锁定、按钮"关闭"
	ui->edit_IP->setEnabled(enabled);
	ui->edit_Port->setEnabled(enabled);
	ui->cmbBox_ProtocolType->setEnabled(enabled);
	ui->Btn_Create->setText(enabled ? tr("打开链接") : tr("关闭链接"));
}

void MainWindow::connectLog()
{
	// 点击清除日志
	connect(ui->Btn_ClearCommLog, &QPushButton::clicked, this, [=]
			{ ui->text_CommLog->clear(); });

	// 日志显示进制切换(Ascii/HEX)
	QButtonGroup *group2 = new QButtonGroup(this);
	group2->addButton(ui->Radio_Log_Ascii);
	group2->addButton(ui->Radio_Log_HEX);
	ui->Radio_Log_Ascii->setChecked(true);
	connect(group2, &QButtonGroup::buttonToggled, this, [this](QAbstractButton *button, bool checked)
			{
		if (checked)
			m_logFormat = (button == ui->Radio_Log_HEX) ? LogFormat::Hex : LogFormat::Ascii;
			});

	// 主控类持有的通信实例信号转发
	if (m_pWorkflow != nullptr)
	{
		// 通信日志记录(纯转发,直连槽)
		connect(m_pWorkflow, &MainWorkflow::logRecord, this, &MainWindow::UpdateLogDisplay);

		// Lua 脚本日志转发(纯转发,直连槽)
		connect(m_pWorkflow->scriptHost(), &ScriptEngineHost::scriptLog, this, &MainWindow::UpdateLogDisplay);

		// 通信事件(收/发统一) — 单点接收,含前缀拼接与重复帧过滤
		connect(m_pWorkflow, &MainWorkflow::commEvent, this, [=](CommEvent ev)
				{
					// 重复帧过滤:与上一帧(端点+数据)完全相同则跳过,避免轮询刷屏;收/发各自独立
					if (ev.direction == CommDirection::eReceive) {
						if (m_lastRecEndpoint == ev.endpointId && m_lastRecData == ev.bytes) return;
						m_lastRecEndpoint = ev.endpointId;
						m_lastRecData     = ev.bytes;
					} else {
						if (m_lastSendEndpoint == ev.endpointId && m_lastSendData == ev.bytes) return;
						m_lastSendEndpoint = ev.endpointId;
						m_lastSendData     = ev.bytes;
					}

					QString body(ev.bytes);
					if (m_logFormat == LogFormat::Hex) body = QString(ev.bytes.toHex().toUpper());

					const QString pfx = (ev.direction == CommDirection::eReceive) ? "Rece" : "Send";
					UpdateLogDisplay(QString("%1:[%2]:").arg(pfx, ev.endpointId) + body);
				});
	}
}

void MainWindow::connectStatusBar()
{
	if (!m_statusBarController) return;
	StatusBarController* sc = m_statusBarController.get();

	// 通信:连接状态 / 客户端快照 / 收发事件 / 超时(经工作流转发)。
	// 这些信号当前均在 GUI 线程发出(Comm 实例以工作流为父、居 GUI 线程),故为直连;
	// 仅 scriptFinished 来自脚本线程池,为跨线程 Queued。
	if (m_pWorkflow)
	{
		connect(m_pWorkflow, &MainWorkflow::connectionStateChanged, sc, &StatusBarController::onConnectionStateChanged);
		connect(m_pWorkflow, &MainWorkflow::clientsChanged,         sc, &StatusBarController::onClientsChanged);
		connect(m_pWorkflow, &MainWorkflow::commEvent,             sc, &StatusBarController::onCommEvent);
		connect(m_pWorkflow, &MainWorkflow::commTimeout,           sc, &StatusBarController::onCommTimeout);
		// 脚本运行态:started/finished 均由 ScriptEngineHost 在唯一执行咽喉发出,严格 1:1 配对
		connect(m_pWorkflow->scriptHost(), &ScriptEngineHost::scriptStarted,  sc, &StatusBarController::onScriptStarted);
		connect(m_pWorkflow->scriptHost(), &ScriptEngineHost::scriptFinished, sc, &StatusBarController::onScriptFinished);
	}

	// 脚本悬浮显示名称框内容(惰性读取,运行中改名亦实时);空名回退 "脚本 #N"
	sc->setScriptNameProvider([this](int i) {
		const QVector<QLineEdit*> edits = scriptNameEdits();
		return (i >= 0 && i < edits.size()) ? edits[i]->text() : QString();
	});

	// 平台位姿(独立于自动写入开关,单独一份连接)
	if (m_simulationPlatform)
		connect(m_simulationPlatform, &SimulationPlatform::poseChanged, sc, &StatusBarController::onPoseChanged);
}

void MainWindow::setupInputValidators()
{
	QIntValidator *PortValid = new QIntValidator(0, 65535, this);
	QIntValidator *RegisterShowAddr = new QIntValidator(0, REGISTER_VAL_NUM - 1, this);

	// 只能输入整型数
	ui->edit_Port->setValidator(PortValid);
	ui->edit_RegisterAddr->setValidator(RegisterShowAddr);

	ui->edit_IP->setInputMask("000.000.000.000;"); // IP地址格式
}

void MainWindow::CreateCurrentProtocol()
{
	int nCurIndex = ui->cmbBox_ProtocolType->currentIndex();

	if (nCurIndex < 0)
		return;

	QVariant data = ui->cmbBox_ProtocolType->itemData(nCurIndex);

	if (m_pWorkflow == nullptr)
		return;

	m_pWorkflow->CreateCommProtocol(data.value<ProtocolType>());
}

// ====================轴位置写入相关槽函数实现====================

bool MainWindow::axisStartAddrValid(int addr) const
{
    // 起址须非负,且留足一整段位姿(kAxisRegSpan 个寄存器)不越界
    return addr >= 0 && addr < REGISTER_VAL_NUM - kAxisRegSpan;
}

void MainWindow::writeAxisPos(int startAddr, PlatformController::NumFormat fmt, Platform which)
{
    m_platformController->writeCurrentPos(
        startAddr,
        startAddr + kAxisFieldStride,
        startAddr + 2 * kAxisFieldStride,
        fmt, which);
}

void MainWindow::writeAxisManual(PlatformController::NumFormat fmt)
{
    if (m_simulationPlatform == nullptr) return;

    int startAddr = m_platformParams.objAddr;
    if (!axisStartAddrValid(startAddr)) { UpdateLogDisplay(tr("错误: 寄存器地址超出范围")); return; }

    writeAxisPos(startAddr, fmt, Platform::Live);
    UpdateLogDisplay(tr("轴位置写入成功 (地址:%1)").arg(startAddr));
}

void MainWindow::OnWriteAxisDoubleWord()
{
    writeAxisManual(PlatformController::NumFormat::Int32);
}

void MainWindow::OnWriteAxisFloat()
{
    writeAxisManual(PlatformController::NumFormat::Float);
}

// ====================自动写入相关槽函数实现====================

void MainWindow::OnPlatformPoseChanged(Platform which, const Pose& pose)
{
    Q_UNUSED(pose);
    if (!m_actAutoWrite->isChecked()) return;
    if (m_simulationPlatform == nullptr) return;

    int startAddr = (which == Platform::Live)
        ? m_platformParams.objAddr
        : m_platformParams.tgtAddr;

    if (!axisStartAddrValid(startAddr)) return;

    // fmtGroup 为 exclusive,两者必有其一选中,无需判"都未选"
    PlatformController::NumFormat fmt = m_actFmtFloat->isChecked()
        ? PlatformController::NumFormat::Float
        : PlatformController::NumFormat::Int32;

    writeAxisPos(startAddr, fmt, which);
}

void MainWindow::OnThemeSelected(Theme theme)
{
	ThemeManager::instance().applyTheme(theme);
	if (m_configStore != nullptr)
	{
		m_configStore->SaveThemePref(static_cast<int>(theme));
	}
}

void MainWindow::OnLanguageSelected(AppLanguage lang)
{
	if (lang == currentLanguage())
		return;

	switchLanguage(*qApp, lang);
	if (m_configStore != nullptr)
		m_configStore->SaveLanguagePref(languageToCode(lang));

	if (m_actLangZh)
		m_actLangZh->setChecked(lang == AppLanguage::Chinese);
	if (m_actLangEn)
		m_actLangEn->setChecked(lang == AppLanguage::English);
}

void MainWindow::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
		retranslateDynamicUi();
	}
	QMainWindow::changeEvent(event);
}

void MainWindow::retranslateDynamicUi()
{
	const QString mainTitle = QStringLiteral("%1 - v%2")
		.arg(QCoreApplication::translate("AppInfo", APP_NAME))
		.arg(APP_VERSION);
	setWindowTitle(mainTitle);

	if (m_subWindow)
		m_subWindow->setWindowTitle(mainTitle + tr(" - 子窗口"));
	if (m_simulationPlatform)
		m_simulationPlatform->setWindowTitle(mainTitle + tr(" - 模拟平台"));

	{
		QMap<ProtocolType, QString> protocolTypeMap;
		protocolTypeMap[ProtocolType::eProRegKeyencePCLink] = tr("基恩士PC-LINK上位链路协议");
		protocolTypeMap[ProtocolType::eProRegMitsubishiQBinary] = tr("三菱MC协议二进制通信");
		for (int i = 0; i < ui->cmbBox_ProtocolType->count(); ++i)
		{
			const auto type = ui->cmbBox_ProtocolType->itemData(i).value<ProtocolType>();
			ui->cmbBox_ProtocolType->setItemText(i, protocolTypeMap.value(type));
		}
	}

	{
		QMap<RegisterDataType, QString> dataTypeMap;
		dataTypeMap[RegisterDataType::eDataTypeChar8] = tr("字符");
		dataTypeMap[RegisterDataType::eDataTypeInt16] = tr("单字");
		dataTypeMap[RegisterDataType::eDataTypeInt32] = tr("双字");
		dataTypeMap[RegisterDataType::eDataTypeFloat] = tr("单精度");
		dataTypeMap[RegisterDataType::eDataTypeDouble] = tr("双精度");
		for (int i = 0; i < ui->cmbBox_DataType->count(); ++i)
		{
			const auto type = ui->cmbBox_DataType->itemData(i).value<RegisterDataType>();
			ui->cmbBox_DataType->setItemText(i, dataTypeMap.value(type));
		}
	}

	setCommControlsEnabled(ui->edit_IP->isEnabled());

	if (m_helpMenu)
		m_helpMenu->setTitle(tr("帮助(&H)"));
	if (m_actAbout)
		m_actAbout->setText(tr("关于(&A)"));
	if (m_actChangelog)
		m_actChangelog->setText(tr("更新日志(&U)"));
	if (m_viewMenu)
		m_viewMenu->setTitle(tr("视图(&V)"));
	if (m_actLightTheme)
		m_actLightTheme->setText(tr("浅色"));
	if (m_actDarkTheme)
		m_actDarkTheme->setText(tr("深色"));
	if (m_platformMenu)
		m_platformMenu->setTitle(tr("平台(&P)"));
	if (m_themeMenu)
		m_themeMenu->setTitle(tr("主题"));
	if (m_langMenu)
		m_langMenu->setTitle(tr("语言"));
	if (m_fmtMenu)
		m_fmtMenu->setTitle(tr("写入格式"));
	if (m_actShowPlatform)
		m_actShowPlatform->setText(tr("显示平台"));
	if (m_actAutoWrite)
		m_actAutoWrite->setText(tr("自动写入轴位置"));
	if (m_actFmtFloat)
		m_actFmtFloat->setText(tr("浮点写入"));
	if (m_actFmtInt32)
		m_actFmtInt32->setText(tr("双字写入"));
	if (m_actPlatformParams)
		m_actPlatformParams->setText(tr("参数设置…"));
	if (m_platformToolBar)
		m_platformToolBar->setWindowTitle(tr("平台操作"));
	if (m_actManualFloat)
	{
		m_actManualFloat->setText(tr("浮点"));
		m_actManualFloat->setToolTip(tr("浮点写入"));
	}
	if (m_actManualInt32)
	{
		m_actManualInt32->setText(tr("双字"));
		m_actManualInt32->setToolTip(tr("双字写入"));
	}

	if (m_registerTableController)
		m_registerTableController->retranslateHeaders();

	if (m_statusBarController)
		m_statusBarController->refreshAll();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	// 小窗最小化/还原时,模拟平台跟随其窗口状态(主窗口隐藏、仅小窗显示的场景)
	if (watched == m_subWindow.get() && event->type() == QEvent::WindowStateChange)
	{
		if (m_subWindow->isVisible())
		{
			m_simulationPlatform->setWindowState(m_subWindow->windowState());
		}
		m_subWindow->activateWindow();
	}

	// 平台窗口显隐 → 回写菜单勾选(QSignalBlocker 防回环)
	if (watched == m_simulationPlatform &&
		(event->type() == QEvent::Show || event->type() == QEvent::Hide))
	{
		if (m_actShowPlatform) {
			QSignalBlocker blocker(m_actShowPlatform);
			m_actShowPlatform->setChecked(m_simulationPlatform->isVisible());
		}
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::UpdateLogDisplay(QString strNewLog)
{
	QTextDocument *document = ui->text_CommLog->document();

	// 获取当前行数;超过最大限制则整体清空
	int lineCount = document->blockCount();
	if (lineCount > kMaxLogLines)
	{
		ui->text_CommLog->clear();
	}

	// 添加新日志，自动滚动到最底部
	ui->text_CommLog->append(strNewLog);
	ui->text_CommLog->moveCursor(QTextCursor::End);
}
