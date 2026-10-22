#include "ConfigStore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QDebug>

namespace {
// 顶层 JSON 键(文件布局,ConfigStore 自身职责;记录内部字段名/类型名由各子系统拥有)
constexpr auto kCommInfo           = "comm_info";
constexpr auto kProtocolType       = "protocol_type";
constexpr auto kTheme              = "theme";
constexpr auto kLanguage           = "language";
constexpr auto kScriptNames        = "script_names";
constexpr auto kSimulationPlatform = "simulation_platform";
constexpr auto kAxisWriteParams    = "axis_write_params";
constexpr auto kRegisterView       = "register_view";
}

ConfigStore::ConfigStore(QObject* parent)
    : QObject(parent)
{
    InitializeConfigDirectory();
    ReadFile();   // 载入 m_root,后续全程内存操作
}

ConfigStore::~ConfigStore() = default;   // 改一项写一次,析构无需再存

bool ConfigStore::InitializeConfigDirectory()
{
    // 获取可执行文件所在目录
    QString appDir = QCoreApplication::applicationDirPath();

    // 构建Config目录路径
    m_configDirPath = appDir + "/Config";
    m_configFilePath = m_configDirPath + "/app_config.json";

    // 创建目录如果不存在
    QDir dir;
    if (!dir.exists(m_configDirPath))
    {
        if (!dir.mkpath(m_configDirPath))
        {
            qWarning() << "Failed to create config directory:" << m_configDirPath;
            return false;
        }
    }

    return true;
}

bool ConfigStore::ReadFile()
{
    QFile file(m_configFilePath);
    if (!file.exists()) { m_root = QJsonObject(); return true; } // 首次运行
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open config file for reading:" << m_configFilePath;
        m_root = QJsonObject(); return false;
    }
    const QByteArray data = file.readAll();
    file.close();
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << err.errorString();
        m_root = QJsonObject(); return false;
    }
    m_root = doc.object();
    return true;
}

bool ConfigStore::WriteFile()
{
    QSaveFile file(m_configFilePath);   // 原子写:写临时文件,commit 时 rename
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open config file for writing:" << m_configFilePath;
        return false;
    }
    file.write(QJsonDocument(m_root).toJson());
    if (!file.commit()) {
        qWarning() << "Failed to commit config file:" << m_configFilePath;
        return false;
    }
    return true;
}

bool ConfigStore::SaveCommInfo(const QVariantMap& record)
{
    return Set(kCommInfo, QJsonObject::fromVariantMap(record));
}

bool ConfigStore::LoadCommInfo(QVariantMap& record) const
{
    if (!m_root.contains(kCommInfo)) return false;
    record = Get(kCommInfo).toObject().toVariantMap();
    return true;
}

bool ConfigStore::SaveProtocolType(int protocolType)
{
    return Set(kProtocolType, protocolType);
}

bool ConfigStore::LoadProtocolType(int& protocolType) const
{
    if (!m_root.contains(kProtocolType)) return false;
    protocolType = Get(kProtocolType).toInt(-1);
    return protocolType != -1;
}

bool ConfigStore::SaveThemePref(int themeId)
{
    return Set(kTheme, themeId);
}

bool ConfigStore::LoadThemePref(int& themeId) const
{
    if (!m_root.contains(kTheme)) return false;
    themeId = Get(kTheme).toInt(themeId);
    return true;
}

bool ConfigStore::SaveLanguagePref(const QString& languageCode)
{
    return Set(kLanguage, languageCode);
}

bool ConfigStore::LoadLanguagePref(QString& languageCode) const
{
    if (!m_root.contains(kLanguage)) return false;
    languageCode = Get(kLanguage).toString();
    return !languageCode.isEmpty();
}

bool ConfigStore::SaveScriptNames(const QStringList& scriptNames)
{
    QJsonArray arr;
    for (const QString& n : scriptNames) arr.append(n);
    return Set(kScriptNames, arr);
}

bool ConfigStore::LoadScriptNames(QStringList& scriptNames) const
{
    if (!m_root.contains(kScriptNames)) return false;
    const QJsonArray arr = Get(kScriptNames).toArray();
    scriptNames.clear();
    for (const QJsonValue& v : arr) scriptNames.append(v.toString());
    return true;
}

bool ConfigStore::SaveSimulationPlatformParams(const QVariantMap& params)
{
    return Set(kSimulationPlatform, QJsonObject::fromVariantMap(params));
}

bool ConfigStore::LoadSimulationPlatformParams(QVariantMap& params) const
{
    if (!m_root.contains(kSimulationPlatform)) return false;
    params = Get(kSimulationPlatform).toObject().toVariantMap();
    return true;
}

bool ConfigStore::SaveAxisWriteParams(const QVariantMap& params)
{
    return Set(kAxisWriteParams, QJsonObject::fromVariantMap(params));
}

bool ConfigStore::LoadAxisWriteParams(QVariantMap& params) const
{
    if (!m_root.contains(kAxisWriteParams)) return false;
    params = Get(kAxisWriteParams).toObject().toVariantMap();
    return true;
}

bool ConfigStore::SaveRegisterView(const QVariantMap& params)
{
    return Set(kRegisterView, QJsonObject::fromVariantMap(params));
}

bool ConfigStore::LoadRegisterView(QVariantMap& params) const
{
    if (!m_root.contains(kRegisterView)) return false;
    params = Get(kRegisterView).toObject().toVariantMap();
    return true;
}
