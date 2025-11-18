#include "ConfigManager.h"
#include "Comm/CommBase.h"
#include "Comm/Socket/CommSocket.h"
#include "MainWorkFlow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
{
    InitializeConfigDirectory();
}

ConfigManager::~ConfigManager()
{
    // 析构时确保所有配置已保存
    SaveAllConfigs();
}

bool ConfigManager::InitializeConfigDirectory()
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

QString ConfigManager::GetConfigFilePath() const
{
    return m_configFilePath;
}

bool ConfigManager::ReadConfigFile()
{
    QFile file(m_configFilePath);
    
    // 如果文件不存在，返回true（认为是首次运行）
    if (!file.exists())
    {
        m_configDoc = QJsonDocument(QJsonObject());
        return true;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open config file for reading:" << m_configFilePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    m_configDoc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "JSON parse error:" << parseError.errorString();
        m_configDoc = QJsonDocument(QJsonObject());
        return false;
    }
    
    return true;
}

bool ConfigManager::WriteConfigFile()
{
    QFile file(m_configFilePath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open config file for writing:" << m_configFilePath;
        return false;
    }
    
    QByteArray data = m_configDoc.toJson();
    qint64 written = file.write(data);
    file.close();
    
    if (written == -1)
    {
        qWarning() << "Failed to write config file:" << m_configFilePath;
        return false;
    }
    
    return true;
}

bool ConfigManager::SaveCommInfo(CommConfig* commInfo)
{
    if (!commInfo)
    {
        qWarning() << "CommInfo is null";
        return false;
    }
    
    if (!ReadConfigFile())
    {
        return false;
    }
    
    QJsonObject root = m_configDoc.object();
    QJsonObject commObj;
    if (!SerializeCommInfoToJson(commInfo, commObj))
    {
        return false;
    }
    root["comm_info"] = commObj;
    m_configDoc.setObject(root);
    
    return WriteConfigFile();
}

bool ConfigManager::LoadCommInfo(std::unique_ptr<CommConfig>& commInfo)
{
    if (!ReadConfigFile())
    {
        return false;
    }
    
    QJsonObject root = m_configDoc.object();
    
    if (!root.contains("comm_info"))
    {
        qWarning() << "No comm_info found in config file";
        return false;
    }
    
    QJsonObject commObj = root["comm_info"].toObject();
    return ParseCommInfoFromJson(commObj, commInfo);
}

bool ConfigManager::SerializeCommInfoToJson(CommConfig* commInfo, QJsonObject& jsonObj)
{
    if (!commInfo)
    {
        return false;
    }
    // 新结构：保存通信类型与通用参数字典
    QString typeStr = (commInfo->type == CommBase::CommType::eSocket) ? "Socket" :
                      (commInfo->type == CommBase::CommType::eSerial) ? "Serial" : "Unknown";
    jsonObj["comm_type"] = typeStr;
    QJsonObject paramsObj;
    for (auto it = commInfo->params.begin(); it != commInfo->params.end(); ++it)
    {
        const QString& key = it.key();
        const QVariant& val = it.value();
        switch (val.typeId()) {
        case QMetaType::Int:
        case QMetaType::UInt:
            paramsObj[key] = val.toInt();
            break;
        case QMetaType::Double:
            paramsObj[key] = val.toDouble();
            break;
        case QMetaType::Bool:
            paramsObj[key] = val.toBool();
            break;
        default:
            paramsObj[key] = val.toString();
            break;
        }
    }
    jsonObj["params"] = paramsObj;
    return true;
}

bool ConfigManager::ParseCommInfoFromJson(const QJsonObject& jsonObj, std::unique_ptr<CommConfig>& commInfo)
{
    QString commType = jsonObj["comm_type"].toString();
    commInfo = std::make_unique<CommConfig>();
    if (commType == "Socket")
    {
        commInfo->type = CommBase::CommType::eSocket;
    }
    else if (commType == "Serial")
    {
        commInfo->type = CommBase::CommType::eSerial;
    }
    else
    {
        commInfo->type = CommBase::CommType::eCommUnknown;
    }
    if (jsonObj.contains("params"))
    {
        QJsonObject paramsObj = jsonObj["params"].toObject();
        for (auto it = paramsObj.begin(); it != paramsObj.end(); ++it)
        {
            commInfo->params.insert(it.key(), it.value().toVariant());
        }
    }
   // commInfo = cfg.release();
    return true;
}

bool ConfigManager::SaveProtocolType(int protocolType)
{
    if (!ReadConfigFile())
    {
        return false;
    }
    
    QJsonObject root = m_configDoc.object();
    root["protocol_type"] = protocolType;
    m_configDoc.setObject(root);
    
    m_cachedConfig.protocolType = protocolType;
    
    return WriteConfigFile();
}

bool ConfigManager::LoadProtocolType(int& protocolType)
{
    if (!ReadConfigFile())
    {
        return false;
    }
    
    QJsonObject root = m_configDoc.object();
    
    if (!root.contains("protocol_type"))
    {
        qWarning() << "No protocol_type found in config file";
        return false;
    }
    
    protocolType = root["protocol_type"].toInt(-1);
    m_cachedConfig.protocolType = protocolType;
    
    return protocolType != -1;
}

bool ConfigManager::SaveScriptNames(const QStringList& scriptNames)
{
    if (scriptNames.size() != 6)
    {
        qWarning() << "Script names list must contain exactly 6 items";
        return false;
    }
    
    if (!ReadConfigFile())
    {
        return false;
    }
    
    QJsonObject root = m_configDoc.object();
    QJsonArray scriptArray;
    
    for (const QString& name : scriptNames)
    {
        scriptArray.append(name);
    }
    
    root["script_names"] = scriptArray;
    m_configDoc.setObject(root);
    
    m_cachedConfig.scriptNames = scriptNames;
    
    return WriteConfigFile();
}

bool ConfigManager::LoadScriptNames(QStringList& scriptNames)
{
    if (!ReadConfigFile())
    {
        return false;
    }
    
    QJsonObject root = m_configDoc.object();
    
    if (!root.contains("script_names"))
    {
        qWarning() << "No script_names found in config file";
        return false;
    }
    
    QJsonArray scriptArray = root["script_names"].toArray();
    
    if (scriptArray.size() != 6)
    {
        qWarning() << "Script names array size is not 6:" << scriptArray.size();
        return false;
    }
    
    scriptNames.clear();
    for (int i = 0; i < scriptArray.size(); ++i)
    {
        scriptNames.append(scriptArray[i].toString());
    }
    
    m_cachedConfig.scriptNames = scriptNames;
    
    return true;
}

bool ConfigManager::SaveSimulationPlatformParams(double markCenterDistance, double screenRatio)
{
    if (!ReadConfigFile())
    {
        return false;
    }
    
    QJsonObject root = m_configDoc.object();
    QJsonObject platformObj;
    
    platformObj["mark_center_distance"] = markCenterDistance;
    platformObj["screen_ratio"] = screenRatio;
    
    root["simulation_platform"] = platformObj;
    m_configDoc.setObject(root);
    
    m_cachedConfig.markCenterDistance = markCenterDistance;
    m_cachedConfig.screenRatio = screenRatio;
    
    return WriteConfigFile();
}

bool ConfigManager::LoadSimulationPlatformParams(double& markCenterDistance, double& screenRatio)
{
    if (!ReadConfigFile())
    {
        return false;
    }
    
    QJsonObject root = m_configDoc.object();
    
    if (!root.contains("simulation_platform"))
    {
        qWarning() << "No simulation_platform found in config file";
        return false;
    }
    
    QJsonObject platformObj = root["simulation_platform"].toObject();
    
    markCenterDistance = platformObj["mark_center_distance"].toDouble(0.0);
    screenRatio = platformObj["screen_ratio"].toDouble(0.0);
    
    m_cachedConfig.markCenterDistance = markCenterDistance;
    m_cachedConfig.screenRatio = screenRatio;
    
    return true;
}

bool ConfigManager::LoadAllConfigs()
{
    bool success = true;
    
    // 尝试加载所有配置，失败时继续尝试其他配置
   /* CommConfig* commInfo = nullptr;*/
    std::unique_ptr<CommConfig> commInfo;
    if (!LoadCommInfo(commInfo))
    {
        qWarning() << "Failed to load comm info";
        success = false;
    }
//     else if (commInfo)
//     {
//         delete commInfo;
//     }
    
    int protocolType = -1;
    if (!LoadProtocolType(protocolType))
    {
        qWarning() << "Failed to load protocol type";
        success = false;
    }
    
    QStringList scriptNames;
    if (!LoadScriptNames(scriptNames))
    {
        qWarning() << "Failed to load script names";
        success = false;
    }
    
    double markCenterDistance = 0.0, screenRatio = 0.0;
    if (!LoadSimulationPlatformParams(markCenterDistance, screenRatio))
    {
        qWarning() << "Failed to load simulation platform params";
        success = false;
    }
    
    return success;
}

bool ConfigManager::SaveAllConfigs()
{
    bool success = true;
    
    // 保存缓存的配置数据
    if (m_cachedConfig.protocolType != -1)
    {
        if (!SaveProtocolType(m_cachedConfig.protocolType))
        {
            qWarning() << "Failed to save protocol type";
            success = false;
        }
    }
    
    if (!m_cachedConfig.scriptNames.isEmpty())
    {
        if (m_cachedConfig.scriptNames.size() == 6)
        {
            if (!SaveScriptNames(m_cachedConfig.scriptNames))
            {
                qWarning() << "Failed to save script names";
                success = false;
            }
        }
    }
    
    if (m_cachedConfig.markCenterDistance > 0 || m_cachedConfig.screenRatio > 0)
    {
        if (!SaveSimulationPlatformParams(m_cachedConfig.markCenterDistance, m_cachedConfig.screenRatio))
        {
            qWarning() << "Failed to save simulation platform params";
            success = false;
        }
    }
    
    return success;
}
