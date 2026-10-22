/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "AuxDialogs.h"
#include "Core/RegisterStore.h"
#include "version.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QPixmap>
#include <QIcon>
#include <QPushButton>
#include <QTextEdit>
#include <QTextStream>
#include <QFile>
#include <QFrame>
#include <QStringConverter>
#include <QCoreApplication>

void AuxDialogs::showAbout(QWidget* parent)
{
    QDialog aboutDialog(parent);
    aboutDialog.setWindowTitle(tr("关于 %1").arg(QCoreApplication::translate("AppInfo", APP_NAME)));
    aboutDialog.setFixedSize(420, 500);
    aboutDialog.setWindowFlags(aboutDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *mainLayout = new QVBoxLayout(&aboutDialog);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 25, 30, 20);

    // 图标显示（居中）
    QLabel *iconLabel = new QLabel(&aboutDialog);
    // .ico 为多帧图标:用 QIcon 按目标尺寸挑最合适的帧;
    // 直接 QPixmap(ico) 只取目录首帧(此处为 16px),放大到 128 会糊。
    QIcon appIcon(":/app/PLC_Simulator.ico");
    iconLabel->setPixmap(appIcon.pixmap(128, 128));
    iconLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(iconLabel);

    // 应用名称（居中）
    QLabel *nameLabel = new QLabel(QCoreApplication::translate("AppInfo", APP_NAME), &aboutDialog);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("font-size: 18pt; font-weight: bold;");
    mainLayout->addWidget(nameLabel);

    // 版本信息（居中）
    QString compileDate = QString::fromLatin1(APP_COMPILE_DATE);
    QString compileTime = QString::fromLatin1(APP_COMPILE_TIME);
    QString versionInfo = QString("Version: %1\nCompile Time: %2 %3\nAuthor: %4")
                              .arg(APP_VERSION)
                              .arg(compileDate)
                              .arg(compileTime)
                              .arg(APP_AUTHOR);

    QLabel *versionLabel = new QLabel(versionInfo, &aboutDialog);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setObjectName("secondaryText");
    mainLayout->addWidget(versionLabel);

    // 分隔线(复用主题细分隔线)
    QFrame *line = new QFrame(&aboutDialog);
    line->setObjectName("hSeparator");
    mainLayout->addWidget(line);

    // 应用描述（靠左）
    QLabel *descLabel = new QLabel(QCoreApplication::translate("AppInfo", APP_DESCRIPTION), &aboutDialog);
    descLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);

    mainLayout->addStretch();

    // 第三方许可按钮(样式跟随全局主题)
    QPushButton *licenseButton = new QPushButton(tr("第三方许可"), &aboutDialog);
    licenseButton->setFixedSize(100, 30);
    QObject::connect(licenseButton, &QPushButton::clicked, &aboutDialog, [parent]() {
        const QString fallback = tr(
            "无法读取第三方许可证文件。\n\n"
            "本软件使用了以下第三方库：\n"
            "1. Qt Framework (LGPL v3)\n"
            "2. Lua 5.4 (MIT License)\n\n"
            "详细信息请查看 THIRD_PARTY_LICENSES.txt 文件。");
        showTextFileDialog(parent, tr("第三方许可证"),
                           QCoreApplication::applicationDirPath() + "/THIRD_PARTY_LICENSES.txt",
                           fallback, QSize(620, 520));
    });

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(licenseButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // 版权信息
    QLabel *copyrightLabel = new QLabel(APP_COPYRIGHT_RC, &aboutDialog);
    QLabel *linkLabel = new QLabel(APP_DOMAIN, &aboutDialog);
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setObjectName("captionText");
    linkLabel->setAlignment(Qt::AlignCenter);
    linkLabel->setObjectName("captionText");
    mainLayout->addWidget(copyrightLabel);
    mainLayout->addWidget(linkLabel);

    aboutDialog.exec();
}

void AuxDialogs::showChangeLog(QWidget* parent)
{
    showTextFileDialog(parent, tr("更新日志"),
                       QCoreApplication::applicationDirPath() + "/ChangeLog.txt",
                       "There is no changeog.", QSize(520, 500));
}

bool AuxDialogs::editPlatformParams(QWidget* parent, PlatformParams& params)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(tr("平台参数设置"));
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QFormLayout* form = new QFormLayout(&dlg);

    QLineEdit* editUnitXY  = new QLineEdit(QString::number(params.unitXY),  &dlg);
    QLineEdit* editUnitD   = new QLineEdit(QString::number(params.unitD),   &dlg);
    QLineEdit* editObjAddr = new QLineEdit(QString::number(params.objAddr), &dlg);
    QLineEdit* editTgtAddr = new QLineEdit(QString::number(params.tgtAddr), &dlg);

    editUnitXY->setAlignment(Qt::AlignCenter);
    editUnitD->setAlignment(Qt::AlignCenter);
    editObjAddr->setAlignment(Qt::AlignCenter);
    editTgtAddr->setAlignment(Qt::AlignCenter);

    editUnitXY->setValidator(new QIntValidator(1, 20, &dlg));
    editUnitD->setValidator(new QIntValidator(1, 20, &dlg));

    const int addrMax = RegisterStore::kRegisterCount - 1 - 6;
    editObjAddr->setValidator(new QIntValidator(0, addrMax, &dlg));
    editTgtAddr->setValidator(new QIntValidator(0, addrMax, &dlg));

    form->addRow(tr("XY单位幂:"), editUnitXY);
    form->addRow(tr("D单位幂:"), editUnitD);
    form->addRow(tr("对象平台轴位置地址:"), editObjAddr);
    form->addRow(tr("目标平台轴位置地址:"), editTgtAddr);

    QDialogButtonBox* box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    box->button(QDialogButtonBox::Ok)->setText(tr("确定"));
    box->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    form->addRow(box);
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return false;

    // 确定:解析写回 params
    params.unitXY  = editUnitXY->text().toInt();
    params.unitD   = editUnitD->text().toInt();
    params.objAddr = editObjAddr->text().toInt();
    params.tgtAddr = editTgtAddr->text().toInt();
    return true;
}

namespace {
// PlatformParams 持久化键(单一来源,to/from 共用)
constexpr auto kUnitXY  = "unitXY";
constexpr auto kUnitD   = "unitD";
constexpr auto kObjAddr = "objAddr";
constexpr auto kTgtAddr = "tgtAddr";
} // namespace

QVariantMap AuxDialogs::PlatformParams::toVariantMap() const
{
    QVariantMap m;
    m[kUnitXY]  = unitXY;
    m[kUnitD]   = unitD;
    m[kObjAddr] = objAddr;
    m[kTgtAddr] = tgtAddr;
    return m;
}

void AuxDialogs::PlatformParams::fromVariantMap(const QVariantMap& m)
{
    // 缺字段回退当前值(= 默认 {3,3,114,120})
    unitXY  = m.value(kUnitXY,  unitXY).toInt();
    unitD   = m.value(kUnitD,   unitD).toInt();
    objAddr = m.value(kObjAddr, objAddr).toInt();
    tgtAddr = m.value(kTgtAddr, tgtAddr).toInt();
}

void AuxDialogs::showTextFileDialog(QWidget* parent, const QString& title,
                                     const QString& filePath, const QString& fallbackText,
                                     const QSize& size)
{
    QFile file(filePath);
    QString content;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        content = in.readAll();
        file.close();
    } else {
        content = fallbackText;
    }

    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(title);
    dialog->setFixedSize(size);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(content);
    layout->addWidget(textEdit);

    QPushButton *closeBtn = new QPushButton(tr("关闭"), dialog);
    closeBtn->setFixedSize(80, 30);
    QObject::connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    dialog->exec();
}
