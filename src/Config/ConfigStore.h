/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class ConfigStore : public QObject
{
    Q_OBJECT

public:
    explicit ConfigStore(QObject* parent = nullptr);
    virtual ~ConfigStore();

    // 禁用拷贝和赋值
    ConfigStore(const ConfigStore&) = delete;
    ConfigStore& operator=(const ConfigStore&) = delete;

    // ==================== 通信参数相关接口 ====================

    // 保存通信信息(不透明字典,序列化由 CommInfoFactory 负责)
    bool SaveCommInfo(const QVariantMap& record);

    // 加载通信信息(不透明字典,反序列化由 CommInfoFactory 负责)
    bool LoadCommInfo(QVariantMap& record) const;

    // ==================== 协议类型相关接口 ====================

    // 保存当前选择的协议类型
    bool SaveProtocolType(int protocolType);

    // 加载协议类型配置
    bool LoadProtocolType(int& protocolType) const;

    // 保存主题偏好(0=浅色, 1=深色)
    bool SaveThemePref(int themeId);

    // 读取主题偏好(无配置时保持入参不变)
    bool LoadThemePref(int& themeId) const;

    // 保存界面语言("zh" / "en")
    bool SaveLanguagePref(const QString& languageCode);

    // 加载界面语言(无配置时保持入参不变)
    bool LoadLanguagePref(QString& languageCode) const;

    // 是否尚无任何已存配置(首次运行;配置文件读取/解析失败亦视为空)
    bool IsEmpty() const { return m_root.isEmpty(); }

    // ==================== 脚本名称相关接口 ====================

    // 保存脚本名称列表(按给定数量与顺序持久化)
    bool SaveScriptNames(const QStringList& scriptNames);

    // 加载脚本名称列表(数量由配置决定;无该键返回 false)
    bool LoadScriptNames(QStringList& scriptNames) const;

    // ==================== 模拟平台参数相关接口 ====================

    // 保存模拟平台参数(不透明字典,字段由 PlatformScene 自描述)
    bool SaveSimulationPlatformParams(const QVariantMap& params);

    // 加载模拟平台参数(不透明字典)
    bool LoadSimulationPlatformParams(QVariantMap& params) const;

    // ==================== 平台/轴写入参数(参数设置对话框)====================

    // 保存轴写入参数(不透明字典,字段由 AuxDialogs::PlatformParams 自描述)
    bool SaveAxisWriteParams(const QVariantMap& params);

    // 加载轴写入参数(不透明字典)
    bool LoadAxisWriteParams(QVariantMap& params) const;

    // ==================== 寄存器表显示设置 ====================

    // 保存寄存器表显示设置(起始地址 / 数据类型 / 进制)
    bool SaveRegisterView(const QVariantMap& params);

    // 加载寄存器表显示设置
    bool LoadRegisterView(QVariantMap& params) const;

private:
    // 配置文件路径 + 内存事实源
    QString m_configDirPath;
    QString m_configFilePath;
    QJsonObject m_root;   // 唯一内存事实源:构造时读入,改一项写一次

    bool InitializeConfigDirectory();
    bool ReadFile();   // 文件 → m_root(仅构造时调用一次);文件不存在视为首次运行
    bool WriteFile();  // m_root → 文件(QSaveFile 原子写)

    // 泛型存取(顶层键)
    QJsonValue Get(const QString& key) const { return m_root.value(key); }
    bool Set(const QString& key, const QJsonValue& value) { m_root.insert(key, value); return WriteFile(); }
};

#endif // CONFIGSTORE_H
