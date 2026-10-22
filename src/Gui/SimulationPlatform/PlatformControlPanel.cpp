/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "PlatformControlPanel.h"
#include "PlatformScene.h"
#include "CollapsibleGroupBox.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QDoubleValidator>
#include <QSignalBlocker>
#include <QEvent>

PlatformControlPanel::PlatformControlPanel(PlatformScene* scene, QWidget* parent)
    : QWidget(parent)
    , m_scene(scene)
{
    buildUi();
    setupValidators();
    wireConnections();

    // 把初始复选框状态写入 scene(保证 scene 与 UI 初值一致)
    // 屏蔽 scene 信号:7 次写入只在末尾统一刷新一次,避免重复 refreshFromScene/重绘
    {
        QSignalBlocker blk(m_scene);
        m_scene->setPlatformVisible(Platform::Base, m_showBaseCheck->isChecked());
        m_scene->setPlatformVisible(Platform::Live, m_showLiveCheck->isChecked());
        m_scene->setBaseMarkVisible(m_showBaseMarkCheck->isChecked());
        m_scene->setLiveMarkVisible(m_showLiveMarkCheck->isChecked());
        m_scene->setVirtualMarkVisible(m_showVirtualMarkCheck->isChecked());
        m_scene->setBaseMarkFollows(m_baseMarkFollowCheck->isChecked());
        m_scene->setLiveMarkFollows(m_liveMarkFollowCheck->isChecked());
    }

    // 同步一次初值
    refreshFromScene();
}

void PlatformControlPanel::buildUi()
{
    // 创建所有输入控件(复选框/分组标题等可翻译文案统一在 retranslateUi 设置)
    m_baseXEdit    = new QLineEdit(this);
    m_baseYEdit    = new QLineEdit(this);
    m_baseAngleEdit = new QLineEdit(this);
    m_showBaseCheck = new QCheckBox(this);
    m_showBaseCheck->setChecked(true);

    m_liveXEdit    = new QLineEdit(this);
    m_liveYEdit    = new QLineEdit(this);
    m_liveAngleEdit = new QLineEdit(this);
    m_showLiveCheck = new QCheckBox(this);
    m_showLiveCheck->setChecked(true);

    m_baseMarkXEdit    = new QLineEdit(this);
    m_baseMarkYEdit    = new QLineEdit(this);
    m_baseMarkAngleEdit = new QLineEdit(this);
    m_baseMarkFollowCheck = new QCheckBox(this);
    m_baseMarkFollowCheck->setChecked(true);
    m_showBaseMarkCheck = new QCheckBox(this);
    m_showBaseMarkCheck->setChecked(true);

    m_liveMarkXEdit    = new QLineEdit(this);
    m_liveMarkYEdit    = new QLineEdit(this);
    m_liveMarkAngleEdit = new QLineEdit(this);
    m_liveMarkFollowCheck = new QCheckBox(this);
    m_liveMarkFollowCheck->setChecked(true);
    m_showLiveMarkCheck = new QCheckBox(this);
    m_showLiveMarkCheck->setChecked(true);

    m_virtualMarkXEdit = new QLineEdit(this);
    m_virtualMarkYEdit = new QLineEdit(this);
    m_showVirtualMarkCheck = new QCheckBox(this);
    m_showVirtualMarkCheck->setChecked(false);  // 虚拟Mark 默认整体关闭

    // 数值输入框统一限宽,居中对齐
    for (QLineEdit* e : { m_baseXEdit, m_baseYEdit, m_baseAngleEdit,
                          m_liveXEdit, m_liveYEdit, m_liveAngleEdit,
                          m_baseMarkXEdit, m_baseMarkYEdit, m_baseMarkAngleEdit,
                          m_liveMarkXEdit, m_liveMarkYEdit, m_liveMarkAngleEdit,
                          m_virtualMarkXEdit, m_virtualMarkYEdit })
    {
        e->setFixedWidth(78);
        e->setAlignment(Qt::AlignCenter);
    }

    // 右对齐表单标签助手
    auto rlbl = [this](const QString& text) {
        QLabel* l = new QLabel(text, this);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return l;
    };
    // 收紧网格行距与内边距
    auto tighten = [](QGridLayout* g) {
        g->setHorizontalSpacing(6);
        g->setVerticalSpacing(4);
        g->setContentsMargins(8, 6, 8, 6);
    };

    // ---- 基准平台 ----
    m_baseGroup = new CollapsibleGroupBox(this);
    {
        QGridLayout* g = new QGridLayout(m_baseGroup); tighten(g); int row = 0;
        g->addWidget(rlbl(QStringLiteral("X:")), row, 0);          g->addWidget(m_baseXEdit, row++, 1);
        g->addWidget(rlbl(QStringLiteral("Y:")), row, 0);          g->addWidget(m_baseYEdit, row++, 1);
        g->addWidget(rlbl(QStringLiteral("Angle (°):")), row, 0);  g->addWidget(m_baseAngleEdit, row++, 1);
        g->addWidget(m_showBaseCheck, row++, 0, 1, 2);
    }

    // ---- 实时平台 ----
    m_liveGroup = new CollapsibleGroupBox(this);
    {
        QGridLayout* g = new QGridLayout(m_liveGroup); tighten(g); int row = 0;
        g->addWidget(rlbl(QStringLiteral("X:")), row, 0);          g->addWidget(m_liveXEdit, row++, 1);
        g->addWidget(rlbl(QStringLiteral("Y:")), row, 0);          g->addWidget(m_liveYEdit, row++, 1);
        g->addWidget(rlbl(QStringLiteral("Angle (°):")), row, 0);  g->addWidget(m_liveAngleEdit, row++, 1);
        g->addWidget(m_showLiveCheck, row++, 0, 1, 2);
    }

    // ---- 基准Mark ----
    m_baseMarkGroup = new CollapsibleGroupBox(this);
    {
        QGridLayout* g = new QGridLayout(m_baseMarkGroup); tighten(g); int row = 0;
        g->addWidget(rlbl(QStringLiteral("X:")), row, 0);          g->addWidget(m_baseMarkXEdit, row++, 1);
        g->addWidget(rlbl(QStringLiteral("Y:")), row, 0);          g->addWidget(m_baseMarkYEdit, row++, 1);
        g->addWidget(rlbl(QStringLiteral("Angle (°):")), row, 0);  g->addWidget(m_baseMarkAngleEdit, row++, 1);
        g->addWidget(m_baseMarkFollowCheck, row, 0);               g->addWidget(m_showBaseMarkCheck, row++, 1);
    }

    // ---- 实时Mark ----
    m_liveMarkGroup = new CollapsibleGroupBox(this);
    {
        QGridLayout* g = new QGridLayout(m_liveMarkGroup); tighten(g); int row = 0;
        g->addWidget(rlbl(QStringLiteral("X:")), row, 0);          g->addWidget(m_liveMarkXEdit, row++, 1);
        g->addWidget(rlbl(QStringLiteral("Y:")), row, 0);          g->addWidget(m_liveMarkYEdit, row++, 1);
        g->addWidget(rlbl(QStringLiteral("Angle (°):")), row, 0);  g->addWidget(m_liveMarkAngleEdit, row++, 1);
        g->addWidget(m_liveMarkFollowCheck, row, 0);               g->addWidget(m_showLiveMarkCheck, row++, 1);
    }

    // ---- 虚拟Mark ----
    m_virtualMarkGroup = new CollapsibleGroupBox(this);
    {
        QGridLayout* g = new QGridLayout(m_virtualMarkGroup); tighten(g);
        m_virtualMarkXLabel = rlbl(QString());
        m_virtualMarkYLabel = rlbl(QString());
        g->addWidget(m_virtualMarkXLabel, 0, 0); g->addWidget(m_virtualMarkXEdit, 0, 1);
        g->addWidget(m_virtualMarkYLabel, 1, 0); g->addWidget(m_virtualMarkYEdit, 1, 1);
        g->addWidget(m_showVirtualMarkCheck, 2, 0, 1, 2);
    }

    // 顶层布局:竖排 5 组 + 弹性空间,本面板即原 rightPanel
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    for (CollapsibleGroupBox* g : { m_baseGroup, m_liveGroup, m_baseMarkGroup, m_liveMarkGroup, m_virtualMarkGroup })
        layout->addWidget(g, 0, Qt::AlignTop);
    layout->addStretch(1);

    retranslateUi();
}

void PlatformControlPanel::retranslateUi()
{
    m_baseGroup->setTitle(tr("基准平台 (mm)"));
    m_liveGroup->setTitle(tr("实时平台 (mm)"));
    m_baseMarkGroup->setTitle(tr("基准Mark (mm)"));
    m_liveMarkGroup->setTitle(tr("实时Mark (mm)"));
    m_virtualMarkGroup->setTitle(tr("虚拟Mark (mm)"));

    for (QCheckBox* c : { m_showBaseCheck, m_showLiveCheck, m_showBaseMarkCheck, m_showLiveMarkCheck, m_showVirtualMarkCheck })
        c->setText(tr("显示"));
    for (QCheckBox* c : { m_baseMarkFollowCheck, m_liveMarkFollowCheck })
        c->setText(tr("跟随平台"));

    m_virtualMarkXLabel->setText(tr("X偏移:"));
    m_virtualMarkYLabel->setText(tr("Y偏移:"));
}

void PlatformControlPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

void PlatformControlPanel::setupValidators()
{
    QDoubleValidator* validator = new QDoubleValidator(this);
    validator->setDecimals(2);

    m_baseXEdit->setValidator(validator);
    m_baseYEdit->setValidator(validator);
    m_baseAngleEdit->setValidator(validator);

    m_liveXEdit->setValidator(validator);
    m_liveYEdit->setValidator(validator);
    m_liveAngleEdit->setValidator(validator);

    m_baseMarkXEdit->setValidator(validator);
    m_baseMarkYEdit->setValidator(validator);
    m_baseMarkAngleEdit->setValidator(validator);

    m_liveMarkXEdit->setValidator(validator);
    m_liveMarkYEdit->setValidator(validator);
    m_liveMarkAngleEdit->setValidator(validator);

    m_virtualMarkXEdit->setValidator(validator);
    m_virtualMarkYEdit->setValidator(validator);
}

void PlatformControlPanel::wireConnections()
{
    // 基准平台
    connect(m_baseXEdit,     &QLineEdit::editingFinished, this, &PlatformControlPanel::pushBaseToScene);
    connect(m_baseYEdit,     &QLineEdit::editingFinished, this, &PlatformControlPanel::pushBaseToScene);
    connect(m_baseAngleEdit, &QLineEdit::editingFinished, this, &PlatformControlPanel::pushBaseToScene);
    connect(m_showBaseCheck, &QCheckBox::clicked, this, [this](bool on) {
        m_scene->setPlatformVisible(Platform::Base, on);
    });

    // 实时平台
    connect(m_liveXEdit,     &QLineEdit::editingFinished, this, &PlatformControlPanel::pushLiveToScene);
    connect(m_liveYEdit,     &QLineEdit::editingFinished, this, &PlatformControlPanel::pushLiveToScene);
    connect(m_liveAngleEdit, &QLineEdit::editingFinished, this, &PlatformControlPanel::pushLiveToScene);
    connect(m_showLiveCheck, &QCheckBox::clicked, this, [this](bool on) {
        m_scene->setPlatformVisible(Platform::Live, on);
    });

    // 基准Mark
    connect(m_baseMarkXEdit,     &QLineEdit::editingFinished, this, &PlatformControlPanel::pushBaseMarkToScene);
    connect(m_baseMarkYEdit,     &QLineEdit::editingFinished, this, &PlatformControlPanel::pushBaseMarkToScene);
    connect(m_baseMarkAngleEdit, &QLineEdit::editingFinished, this, &PlatformControlPanel::pushBaseMarkToScene);
    connect(m_baseMarkFollowCheck, &QCheckBox::clicked, this, [this](bool on) {
        m_scene->setBaseMarkFollows(on);
    });
    connect(m_showBaseMarkCheck, &QCheckBox::clicked, this, [this](bool on) {
        m_scene->setBaseMarkVisible(on);
    });

    // 实时Mark
    connect(m_liveMarkXEdit,     &QLineEdit::editingFinished, this, &PlatformControlPanel::pushLiveMarkToScene);
    connect(m_liveMarkYEdit,     &QLineEdit::editingFinished, this, &PlatformControlPanel::pushLiveMarkToScene);
    connect(m_liveMarkAngleEdit, &QLineEdit::editingFinished, this, &PlatformControlPanel::pushLiveMarkToScene);
    connect(m_liveMarkFollowCheck, &QCheckBox::clicked, this, [this](bool on) {
        m_scene->setLiveMarkFollows(on);
    });
    connect(m_showLiveMarkCheck, &QCheckBox::clicked, this, [this](bool on) {
        m_scene->setLiveMarkVisible(on);
    });

    // 虚拟Mark
    connect(m_virtualMarkXEdit, &QLineEdit::editingFinished, this, &PlatformControlPanel::pushVirtualMarkToScene);
    connect(m_virtualMarkYEdit, &QLineEdit::editingFinished, this, &PlatformControlPanel::pushVirtualMarkToScene);
    connect(m_showVirtualMarkCheck, &QCheckBox::clicked, this, [this](bool on) {
        m_scene->setVirtualMarkVisible(on);
    });

    // scene 变化 → 回填输入框
    connect(m_scene, &PlatformScene::changed, this, &PlatformControlPanel::refreshFromScene);
}

void PlatformControlPanel::pushBaseToScene()
{
    m_scene->moveAbsolute({ m_baseXEdit->text().toDouble(),
                            m_baseYEdit->text().toDouble(),
                            m_baseAngleEdit->text().toDouble() }, Platform::Base);
}

void PlatformControlPanel::pushLiveToScene()
{
    m_scene->moveAbsolute({ m_liveXEdit->text().toDouble(),
                            m_liveYEdit->text().toDouble(),
                            m_liveAngleEdit->text().toDouble() }, Platform::Live);
}

void PlatformControlPanel::pushBaseMarkToScene()
{
    // 仅写 pose;followsPlatform 由 follow 复选框的 clicked 直接写入(避免与 refreshFromScene 重入冲突)
    m_scene->setBaseMarkPose({ m_baseMarkXEdit->text().toDouble(),
                               m_baseMarkYEdit->text().toDouble(),
                               m_baseMarkAngleEdit->text().toDouble() });
}

void PlatformControlPanel::pushLiveMarkToScene()
{
    // 仅写 pose;followsPlatform 由 follow 复选框的 clicked 直接写入(避免与 refreshFromScene 重入冲突)
    m_scene->setLiveMarkPose({ m_liveMarkXEdit->text().toDouble(),
                               m_liveMarkYEdit->text().toDouble(),
                               m_liveMarkAngleEdit->text().toDouble() });
}

void PlatformControlPanel::pushVirtualMarkToScene()
{
    m_scene->setVirtualMarkOffset(m_virtualMarkXEdit->text().toDouble(),
                                  m_virtualMarkYEdit->text().toDouble());
}

void PlatformControlPanel::refreshFromScene()
{
    const Pose b = m_scene->pose(Platform::Base);
    const Pose l = m_scene->pose(Platform::Live);

    auto setTxt = [](QLineEdit* e, double v) {
        QSignalBlocker blk(e);
        e->setText(QString::number(v, 'f', 2));
    };

    setTxt(m_baseXEdit, b.x);
    setTxt(m_baseYEdit, b.y);
    setTxt(m_baseAngleEdit, b.angleDeg);

    setTxt(m_liveXEdit, l.x);
    setTxt(m_liveYEdit, l.y);
    setTxt(m_liveAngleEdit, l.angleDeg);

    // Mark 输入框 / 复选框也回填(输入框经 setTxt 屏蔽信号;复选框见下方说明)
    const MarkItem& bm = m_scene->baseMark();
    const MarkItem& lm = m_scene->liveMark();
    const MarkItem& vm = m_scene->virtualMark();

    setTxt(m_baseMarkXEdit, bm.pose.x);
    setTxt(m_baseMarkYEdit, bm.pose.y);
    setTxt(m_baseMarkAngleEdit, bm.pose.angleDeg);
    // 复选框均以 clicked 连接(仅响应用户点击),程序化 setChecked 不发信号,故无需 QSignalBlocker。
    // 注意:若日后改为 toggled/stateChanged,必须在此恢复信号屏蔽,否则会重现"跟随被即时回退"的反馈环。
    m_baseMarkFollowCheck->setChecked(bm.followsPlatform);
    m_showBaseMarkCheck->setChecked(bm.visible);

    setTxt(m_liveMarkXEdit, lm.pose.x);
    setTxt(m_liveMarkYEdit, lm.pose.y);
    setTxt(m_liveMarkAngleEdit, lm.pose.angleDeg);
    m_liveMarkFollowCheck->setChecked(lm.followsPlatform);
    m_showLiveMarkCheck->setChecked(lm.visible);

    setTxt(m_virtualMarkXEdit, vm.pose.x);
    setTxt(m_virtualMarkYEdit, vm.pose.y);
    m_showVirtualMarkCheck->setChecked(vm.visible);

    // 平台可见性复选框(同上,clicked 连接,无需信号屏蔽)
    m_showBaseCheck->setChecked(m_scene->platform(Platform::Base).visible);
    m_showLiveCheck->setChecked(m_scene->platform(Platform::Live).visible);
}
