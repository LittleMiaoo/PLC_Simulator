/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */
#include "PlatformParamsDialog.h"
#include "PlatformScene.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QDialogButtonBox>

PlatformParamsDialog::PlatformParamsDialog(PlatformScene* scene, QWidget* parent)
    : QDialog(parent)
    , m_scene(scene)
{
    // 进对话框前记录原始值,供 Cancel 回退
    const double origSpacing = m_scene->markCenterDistance();
    const double origRatio   = m_scene->screenRatio();

    setWindowTitle(tr("参数设置"));

    QLineEdit* sizeEdit  = new QLineEdit(this);
    QLineEdit* ratioEdit = new QLineEdit(this);
    sizeEdit->setText(QString::number(origSpacing));
    ratioEdit->setText(QString::number(origRatio));
    sizeEdit->setAlignment(Qt::AlignCenter);
    ratioEdit->setAlignment(Qt::AlignCenter);
    QDoubleValidator* v = new QDoubleValidator(this);
    v->setDecimals(2);
    sizeEdit->setValidator(v);
    ratioEdit->setValidator(v);

    // 实时预览:写 scene（scene.changed 自动驱动画布重绘；状态栏由外部 scene.changed 连接刷新）
    auto applyPreview = [this](double distance, double ratio) {
        m_scene->setSceneParams(distance, ratio);
    };

    // 用户输入时实时预览;护栏:数值有效且缩放比 > 0
    auto onEdited = [=]() {
        bool okD = false, okR = false;
        const double d = sizeEdit->text().toDouble(&okD);
        const double r = ratioEdit->text().toDouble(&okR);
        if (okD && okR && r > 0.0)
            applyPreview(d, r);
    };
    connect(sizeEdit,  &QLineEdit::textEdited, this, [=](const QString&) { onEdited(); });
    connect(ratioEdit, &QLineEdit::textEdited, this, [=](const QString&) { onEdited(); });

    QFormLayout* form = new QFormLayout();
    form->addRow(tr("产品尺寸 (mm):"), sizeEdit);
    form->addRow(tr("缩放比 (px/mm):"), ratioEdit);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->addLayout(form);
    lay->addWidget(box);

    // Cancel/关闭时回退原值；OK 时 scene 已是新值，由调用方（门面）负责落盘
    connect(this, &QDialog::rejected, this, [this, origSpacing, origRatio]() {
        m_scene->setSceneParams(origSpacing, origRatio);
    });
}
