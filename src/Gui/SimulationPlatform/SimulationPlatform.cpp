/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "SimulationPlatform.h"
#include "PlatformCanvas.h"
#include "PlatformControlPanel.h"
#include "PlatformScene.h"
#include "PlatformParamsDialog.h"
#include "CollapsibleGroupBox.h"
#include "ImagePage.h"
#include "ThemeManager.h"

#include <QStackedWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QDialog>
#include <QRect>
#include <QGuiApplication>
#include <QEvent>
#include <QScreen>

namespace
{
// 窗口位置菜单项(下标即 moveToScreenCorner 的 corner);QT_TRANSLATE_NOOP 供 lupdate 提取,构建与语言切换共用
const char* const kCornerNames[] = {
    QT_TRANSLATE_NOOP("SimulationPlatform", "左上"),
    QT_TRANSLATE_NOOP("SimulationPlatform", "右上"),
    QT_TRANSLATE_NOOP("SimulationPlatform", "左下"),
    QT_TRANSLATE_NOOP("SimulationPlatform", "右下"),
    QT_TRANSLATE_NOOP("SimulationPlatform", "居中"),
};

// "模拟平台"菜单中控制面板分组的开关项(顺序同 buildMenuBar 中 bindGroupToggle 调用),构建与语言切换共用
const char* const kGroupToggleTitles[] = {
    QT_TRANSLATE_NOOP("SimulationPlatform", "基准平台"),
    QT_TRANSLATE_NOOP("SimulationPlatform", "实时平台"),
    QT_TRANSLATE_NOOP("SimulationPlatform", "基准Mark"),
    QT_TRANSLATE_NOOP("SimulationPlatform", "实时Mark"),
    QT_TRANSLATE_NOOP("SimulationPlatform", "虚拟Mark"),
};
} // namespace

SimulationPlatform::SimulationPlatform(QWidget *parent)
    : QMainWindow(parent)
    , m_simMenu(nullptr)
    , m_imageMenu(nullptr)
    , m_statusLeft(nullptr)
    , m_statusRight(nullptr)
{
    // poseChanged 携带 Pose/Platform;注册元类型以支持将来跨线程队列连接(与 CommEvent 一致)
    qRegisterMetaType<Pose>("Pose");
    qRegisterMetaType<Platform>("Platform");

    m_scene = new PlatformScene(this);

    // 转发场景的 poseChanged 信号到外部
    connect(m_scene, &PlatformScene::poseChanged, this, &SimulationPlatform::poseChanged);

    setupUI();

    // 主题切换时实时重绘画布与图像区(控件由全局 qss 自动重绘)
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) {
        if (m_canvas) m_canvas->update();
        if (m_imagePage) m_imagePage->update();
        update();
    });

    setWindowTitle(tr("模拟平台"));
    resize(800, 600);
}

// 平台控制公共接口实现
void SimulationPlatform::moveAbsolute(const Pose& target, Platform which)
{
    m_scene->moveAbsolute(target, which);
}

void SimulationPlatform::moveRelative(const Pose& delta, Platform which)
{
    m_scene->moveRelative(delta, which);
}

Pose SimulationPlatform::pose(Platform which) const
{
    return m_scene->pose(which);
}

void SimulationPlatform::setSceneParams(double markCenterDistance, double screenRatio)
{
    m_scene->setSceneParams(markCenterDistance, screenRatio);
    emit sceneParamsChanged(markCenterDistance, screenRatio);
    update();
}

QVariantMap SimulationPlatform::sceneParamsToMap() const
{
    return m_scene->toVariantMap();
}

void SimulationPlatform::setSceneParamsFromMap(const QVariantMap& params)
{
    m_scene->fromVariantMap(params);
    // 沿用 setSceneParams 的对外通知(状态栏/落盘)与重绘
    emit sceneParamsChanged(m_scene->markCenterDistance(), m_scene->screenRatio());
    update();
}

void SimulationPlatform::setupUI()
{
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // ========== 模拟平台页面 ==========
    m_simulationPage = new QWidget(this);
    QVBoxLayout *simPageLayout = new QVBoxLayout(m_simulationPage);  // 上:收起按钮栏  下:画布+控制面板
    simPageLayout->setContentsMargins(9, 6, 9, 9);  // 四周留白,避免画布贴左、面板贴右

    // 顶栏:收起/展开按钮(右对齐,提到画布与面板的共同上方;折叠后画布全宽时按钮仍在右上角)
    m_panelToggleBtn = new QPushButton(tr("收起 »"), this);
    QHBoxLayout* topBar = new QHBoxLayout();
    topBar->addStretch(1);
    topBar->addWidget(m_panelToggleBtn);
    simPageLayout->addLayout(topBar);

    QHBoxLayout *simLayout = new QHBoxLayout();  // 左:画布  右:控制面板
    simPageLayout->addLayout(simLayout, 1);

    // 画布(占左侧主区)
    m_canvas = new PlatformCanvas(m_scene, this);
    simLayout->addWidget(m_canvas, 1);

    // 右侧控制面板
    m_controlPanel = new PlatformControlPanel(m_scene, this);
    simLayout->addWidget(m_controlPanel);

    // 顶栏按钮收起/展开整个右面板:折叠后画布水平铺满
    connect(m_panelToggleBtn, &QPushButton::clicked, this, [=]() {
        const bool show = !m_controlPanel->isVisible();
        m_controlPanel->setVisible(show);
        m_panelToggleBtn->setText(show ? tr("收起 »") : tr("« 展开"));
    });

    // 模拟平台页加入栈
    m_stack->addWidget(m_simulationPage);   // index 0

    // 图片显示页
    m_imagePage = new ImagePage(this);
    m_stack->addWidget(m_imagePage);      // index 1

    // 菜单栏(视图/模拟平台/图像)
    buildMenuBar();

    // 状态栏:左 = 上下文文本(路径/参数),右 = 缩放百分比
    m_statusLeft  = new QLabel(this);
    m_statusRight = new QLabel(this);
    statusBar()->addWidget(m_statusLeft, 1);
    statusBar()->addPermanentWidget(m_statusRight);

    // 图片缩放变化 → 状态栏右侧百分比(仅图片页显示)
    connect(m_imagePage, &ImagePage::scaleChanged, this, [this](double scale) {
        if (m_stack->currentIndex() == 1)
            m_statusRight->setText(tr("缩放: %1%").arg(QString::number(scale * 100.0, 'f', 0)));
    });

    // 图片路径变化 → 状态栏左侧路径更新(仅图片页显示)
    connect(m_imagePage, &ImagePage::imagePathChanged, this, [this](const QString& path) {
        if (m_stack->currentIndex() == 1)
            m_statusLeft->setText(path.isEmpty() ? tr("未加载图片") : path);
    });

    // 参数变化 → 模拟页状态栏文本刷新
    connect(this, &SimulationPlatform::sceneParamsChanged, this, [this](double, double) {
        if (m_stack->currentIndex() == 0)
            updateStatusBarForPage(0);
    });

    // 场景数据变化 → 模拟页状态栏实时刷新（预览期对话框修改场景时同步更新）
    connect(m_scene, &PlatformScene::changed, this, [this]() {
        if (m_stack->currentIndex() == 0)
            updateStatusBarForPage(0);
    });

    // 默认显示模拟页
    showPage(0);
}

void SimulationPlatform::buildMenuBar()
{
    QMenuBar* mbar = menuBar();

    // ===== 视图 =====
    QMenu* viewMenu = mbar->addMenu(tr("视图"));
    m_viewMenu = viewMenu;

    QActionGroup* pageGroup = new QActionGroup(this);
    pageGroup->setExclusive(true);
    m_actPageSim = viewMenu->addAction(tr("模拟平台"));
    m_actPagePic = viewMenu->addAction(tr("图片显示"));
    for (QAction* a : { m_actPageSim, m_actPagePic }) { a->setCheckable(true); pageGroup->addAction(a); }
    m_actPageSim->setChecked(true);
    connect(m_actPageSim, &QAction::triggered, this, [this]() { showPage(0); });
    connect(m_actPagePic, &QAction::triggered, this, [this]() { showPage(1); });

    viewMenu->addSeparator();

    // 窗口位置(四角,单选,两页共用)
    QMenu* posMenu = viewMenu->addMenu(tr("窗口位置"));
    m_posMenu = posMenu;
    QActionGroup* posGroup = new QActionGroup(this);
    posGroup->setExclusive(true);
    for (int i = 0; i < 5; ++i)
    {
        QAction* a = posMenu->addAction(tr(kCornerNames[i]));
        m_cornerPosActions[i] = a;
        a->setCheckable(true);
        posGroup->addAction(a);
        if (i == 0) a->setChecked(true);
        connect(a, &QAction::triggered, this, [this, i]() { moveToScreenCorner(i); });
    }

    // ===== 模拟平台(仅模拟页可用)=====
    m_simMenu = mbar->addMenu(tr("模拟平台"));
    m_actParam = m_simMenu->addAction(tr("参数设置…"));
    connect(m_actParam, &QAction::triggered, this, &SimulationPlatform::openParamDialog);
    m_simMenu->addSeparator();
    bindGroupToggle(m_controlPanel->baseGroup(),        tr(kGroupToggleTitles[0]));
    bindGroupToggle(m_controlPanel->liveGroup(),        tr(kGroupToggleTitles[1]));
    bindGroupToggle(m_controlPanel->baseMarkGroup(),    tr(kGroupToggleTitles[2]));
    bindGroupToggle(m_controlPanel->liveMarkGroup(),    tr(kGroupToggleTitles[3]));
    bindGroupToggle(m_controlPanel->virtualMarkGroup(), tr(kGroupToggleTitles[4]), false);  // 虚拟Mark 默认不显示

    // ===== 图像(仅图片页可用)=====
    m_imageMenu = mbar->addMenu(tr("图像"));
    m_actLoadImage = m_imageMenu->addAction(tr("加载图片…"));
    connect(m_actLoadImage, &QAction::triggered, m_imagePage, &ImagePage::loadImage);
}

void SimulationPlatform::showPage(int index)
{
    // 切页前记住离开页的窗口尺寸(仅本会话,不落盘)
    const int prev = m_stack->currentIndex();
    if (prev != index && prev >= 0 && prev < 2)
        m_pageSize[prev] = size();

    m_stack->setCurrentIndex(index);

    // 让窗口最小尺寸只受当前页约束:非当前页 sizePolicy 设为 Ignored,
    // 其 qSmartMinSize 归 0,QStackedLayout 不再用它钳制最小高度。
    for (int i = 0; i < m_stack->count(); ++i)
    {
        const bool cur = (i == index);
        m_stack->widget(i)->setSizePolicy(cur ? QSizePolicy::Preferred : QSizePolicy::Ignored,
                                        cur ? QSizePolicy::Preferred : QSizePolicy::Ignored);
    }

    if (index == 0 && m_actPageSim) m_actPageSim->setChecked(true);
    if (index == 1 && m_actPagePic) m_actPagePic->setChecked(true);

    // 页面专属菜单互斥启用:模拟页→模拟平台菜单可用、图像菜单置灰;反之亦然
    if (m_simMenu)   m_simMenu->menuAction()->setEnabled(index == 0);
    if (m_imageMenu) m_imageMenu->menuAction()->setEnabled(index == 1);

    updateStatusBarForPage(index);

    // 恢复目标页此前在本会话记住的尺寸(首次则保持当前自然尺寸)
    if (index >= 0 && index < 2 && m_pageSize[index].isValid())
        resize(m_pageSize[index]);
}

void SimulationPlatform::updateStatusBarForPage(int index)
{
    if (!m_statusLeft || !m_statusRight)
        return;

    if (index == 0)
    {
        // 模拟页:产品尺寸 + 缩放比
        m_statusLeft->setText(tr("产品尺寸: %1 mm    缩放比: %2 px/mm")
                                .arg(QString::number(m_scene->markCenterDistance()))
                                .arg(QString::number(m_scene->screenRatio())));
        m_statusRight->clear();
    }
    else
    {
        // 图片页:路径 + 缩放百分比
        const QString path = m_imagePage ? m_imagePage->imagePath() : QString();
        m_statusLeft->setText(path.isEmpty() ? tr("未加载图片") : path);
        if (m_imagePage)
            m_statusRight->setText(tr("缩放: %1%")
                                     .arg(QString::number(m_imagePage->scale() * 100.0, 'f', 0)));
    }
}

void SimulationPlatform::moveToScreenCorner(int corner)
{
    const QRect fg = this->frameGeometry();
    const int totalWidth  = fg.width();
    const int totalHeight = fg.height();
    const QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
    switch (corner)
    {
    case 0: this->move(0, 0); break;                                              // 左上
    case 1: this->move(avail.width() - totalWidth, 0); break;                     // 右上
    case 2: this->move(0, avail.height() - totalHeight); break;                   // 左下
    case 3: this->move(avail.width() - totalWidth, avail.height() - totalHeight); break; // 右下
    case 4: this->move(avail.x() + (avail.width() - totalWidth) / 2,
                       avail.y() + (avail.height() - totalHeight) / 2); break;     // 居中
    default: break;
    }
}

void SimulationPlatform::bindGroupToggle(CollapsibleGroupBox* g, const QString& title, bool visible)
{
    QAction* a = m_simMenu->addAction(title);
    m_groupToggleActions.push_back(a);
    a->setCheckable(true);
    a->setChecked(visible);   // 初始勾选状态(connect 前设置,不触发)
    g->setVisible(visible);   // 同步初始可见性

    // 菜单勾选 → 显示/隐藏整组
    connect(a, &QAction::toggled, this, [g](bool on) { g->setVisible(on); });

    // 组内 × 关闭 → 取消勾选(toggled 再驱动隐藏)
    g->setClosable(true);
    connect(g, &CollapsibleGroupBox::closed, this, [a]() { a->setChecked(false); });
}

void SimulationPlatform::openParamDialog()
{
    PlatformParamsDialog dlg(m_scene, this);
    if (dlg.exec() == QDialog::Accepted)
        emit sceneParamsChanged(m_scene->markCenterDistance(), m_scene->screenRatio());
    if (m_stack->currentIndex() == 0)
        updateStatusBarForPage(0);
}

void SimulationPlatform::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QMainWindow::changeEvent(event);
}

void SimulationPlatform::retranslateUi()
{
    if (m_viewMenu)
        m_viewMenu->setTitle(tr("视图"));
    if (m_actPageSim)
        m_actPageSim->setText(tr("模拟平台"));
    if (m_actPagePic)
        m_actPagePic->setText(tr("图片显示"));
    if (m_posMenu)
        m_posMenu->setTitle(tr("窗口位置"));

    for (int i = 0; i < 5; ++i) {
        if (m_cornerPosActions[i])
            m_cornerPosActions[i]->setText(tr(kCornerNames[i]));
    }

    if (m_simMenu)
        m_simMenu->setTitle(tr("模拟平台"));
    if (m_actParam)
        m_actParam->setText(tr("参数设置…"));

    for (int i = 0; i < m_groupToggleActions.size() && i < 5; ++i) {
        if (m_groupToggleActions[i])
            m_groupToggleActions[i]->setText(tr(kGroupToggleTitles[i]));
    }

    if (m_imageMenu)
        m_imageMenu->setTitle(tr("图像"));
    if (m_actLoadImage)
        m_actLoadImage->setText(tr("加载图片…"));

    if (m_panelToggleBtn) {
        const bool show = m_controlPanel && m_controlPanel->isVisible();
        m_panelToggleBtn->setText(show ? tr("收起 »") : tr("« 展开"));
    }

    if (m_stack)
        updateStatusBarForPage(m_stack->currentIndex());
}

