#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <memory>
#include "Comm/CommBase.h"

struct CommConfig;

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(QObject* parent = nullptr);
    virtual ~ConfigManager();

    // 禁用拷贝和赋值
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // ==================== 通信参数相关接口 ====================
    
    /**
     * @brief 保存通信参数配置
     * @param commInfo 通信参数基类指针，支持多态保存
     * @return 保存是否成功
     */
    bool SaveCommInfo(CommConfig* commInfo);

    /**
     * @brief 加载通信参数配置
     * @param commInfo 用于存储加载结果的通信参数基类指针引用
     * @return 加载是否成功
     */
    bool LoadCommInfo(CommConfig*& commInfo);

    // ==================== 协议类型相关接口 ====================
    
    /**
     * @brief 保存当前选择的协议类型
     * @param protocolType 协议类型（整数表示）
     * @return 保存是否成功
     */
    bool SaveProtocolType(int protocolType);

    /**
     * @brief 加载协议类型配置
     * @param protocolType 用于存储加载结果的协议类型引用
     * @return 加载是否成功
     */
    bool LoadProtocolType(int& protocolType);

    // ==================== 脚本名称相关接口 ====================
    
    /**
     * @brief 保存所有脚本名称（ScriptName_1至ScriptName_6）
     * @param scriptNames 包含6个脚本名称的字符串列表
     * @return 保存是否成功
     */
    bool SaveScriptNames(const QStringList& scriptNames);

    /**
     * @brief 加载脚本名称配置
     * @param scriptNames 用于存储加载结果的字符串列表引用
     * @return 加载是否成功，返回的列表大小为6
     */
    bool LoadScriptNames(QStringList& scriptNames);

    // ==================== 模拟平台参数相关接口 ====================
    
    /**
     * @brief 保存模拟平台的参数
     * @param markCenterDistance Mark中心间距参数
     * @param screenRatio 屏幕比例参数
     * @return 保存是否成功
     */
    bool SaveSimulationPlatformParams(double markCenterDistance, double screenRatio);

    /**
     * @brief 加载模拟平台参数
     * @param markCenterDistance 用于存储Mark中心间距的引用
     * @param screenRatio 用于存储屏幕比例的引用
     * @return 加载是否成功
     */
    bool LoadSimulationPlatformParams(double& markCenterDistance, double& screenRatio);

    // ==================== 自动保存和加载接口 ====================
    
    /**
     * @brief 加载所有配置（应在软件启动时调用）
     * @return 加载是否成功
     */
    bool LoadAllConfigs();

    /**
     * @brief 保存所有配置（可在应用关闭时调用以确保所有配置已保存）
     * @return 保存是否成功
     */
    bool SaveAllConfigs();

private:
    // 配置文件路径和JSON文档
    QString m_configDirPath;
    QString m_configFilePath;
    QJsonDocument m_configDoc;

    // 配置缓存数据
    struct CachedConfig {
        // 通信参数
        int commType = -1;
        QString commSocketType;
        QString commIPAddress;
        uint32_t commPort = 0;
        uint32_t commListenNum = 0;
        QString commStop;
        QString cmdStop;

        // 协议类型
        int protocolType = -1;

        // 脚本名称
        QStringList scriptNames;

        // 模拟平台参数
        double markCenterDistance = 0.0;
        double screenRatio = 0.0;
    } m_cachedConfig;

    // ==================== 私有辅助方法 ====================
    
    /**
     * @brief 初始化配置目录，确保路径存在
     * @return 初始化是否成功
     */
    bool InitializeConfigDirectory();

    /**
     * @brief 读取JSON配置文件
     * @return 读取是否成功
     */
    bool ReadConfigFile();

    /**
     * @brief 写入JSON配置文件
     * @return 写入是否成功
     */
    bool WriteConfigFile();

    /**
     * @brief 从JSON对象中解析通信参数信息
     * @param jsonObj JSON对象
     * @param commInfo 用于存储结果的通信参数基类指针引用
     * @return 解析是否成功
     */
    bool ParseCommInfoFromJson(const QJsonObject& jsonObj, CommConfig*& commInfo);

    /**
     * @brief 将通信参数信息序列化为JSON对象
     * @param commInfo 通信参数基类指针
     * @param jsonObj 用于存储结果的JSON对象引用
     * @return 序列化是否成功
     */
    bool SerializeCommInfoToJson(CommConfig* commInfo, QJsonObject& jsonObj);

    /**
     * @brief 获取配置文件的完整路径
     * @return 配置文件路径
     */
    QString GetConfigFilePath() const;
};

#endif // CONFIGMANAGER_H
