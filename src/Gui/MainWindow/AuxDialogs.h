/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef AUXDIALOGS_H
#define AUXDIALOGS_H

#include <QCoreApplication>
#include <QString>
#include <QSize>
#include <QVariantMap>

class QWidget;

/**
 * @brief 主界面辅助对话框(关于 / 更新日志 / 平台参数)
 *
 * 无状态,纯 UI 构造。20260626 从 MainWindow.cpp 抽出。
 */
class AuxDialogs
{
    Q_DECLARE_TR_FUNCTIONS(AuxDialogs)   // 非 QObject 类提供 tr()(翻译上下文 "AuxDialogs")

public:
    /// 平台参数包:单位幂次与轴写入起始地址
    struct PlatformParams {
        int unitXY;   ///< XY轴单位幂(10^n)
        int unitD;    ///< D轴单位幂(10^n)
        int objAddr;  ///< 对象平台轴位置起始寄存器地址
        int tgtAddr;  ///< 目标平台轴位置起始寄存器地址

        /// 持久化自序列化(键自描述);fromVariantMap 缺字段回退当前值
        QVariantMap toVariantMap() const;
        void fromVariantMap(const QVariantMap& m);
    };

    /// 关于对话框(图标/版本/作者/描述 + 第三方许可子弹窗)
    static void showAbout(QWidget* parent);

    /// 更新日志对话框
    static void showChangeLog(QWidget* parent);

    /// 平台参数设置对话框(事务式):用 params 做初值;确定写回 params 返 true,取消返 false(params 不变)
    static bool editPlatformParams(QWidget* parent, PlatformParams& params);

private:
    /// 读取文本文件 → 只读 QTextEdit 弹窗;读失败时显示 fallbackText
    static void showTextFileDialog(QWidget* parent, const QString& title,
                                   const QString& filePath, const QString& fallbackText,
                                   const QSize& size);
};

#endif // AUXDIALOGS_H
