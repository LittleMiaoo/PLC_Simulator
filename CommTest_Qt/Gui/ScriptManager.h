#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <QObject>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QCoreApplication>

#include "ScriptEditor/ScriptEditor.h"
#include "../MainFlow/MainWorkFlow.h"

/**
 * 20251225 wm 从CommTest_Qt.cpp拆分
 * @brief 脚本管理器
 *
 * 负责管理Lua脚本的执行、编辑和循环控制。
 * 从 CommTest_Qt 中拆分出来，专注于脚本相关逻辑。
 */
class ScriptManager : public QObject
{
    Q_OBJECT

signals:
    /**
     * @brief 执行Lua脚本信号
     * @param luaIndex 脚本索引
     * @param luaPath 脚本路径
     */
    void executeLuaScript(int luaIndex, QString luaPath);

public:
    /**
     * @brief 构造函数
     * @param workFlow 工作流指针
     * @param parent 父窗口
     */
    explicit ScriptManager(MainWorkFlow* workFlow, QWidget* parent = nullptr);

    ~ScriptManager() override;

    /**
     * @brief 连接执行按钮
     * @param index 脚本索引 (0-5)
     * @param button 执行按钮
     */
    void connectExecuteButton(int index, QPushButton* button);

    /**
     * @brief 连接编辑按钮
     * @param scriptIndex 脚本编号 (1-6)
     * @param button 编辑按钮
     */
    void connectEditButton(int scriptIndex, QPushButton* button);

    /**
     * @brief 连接循环使能复选框
     * @param index 脚本索引 (0-5)
     * @param checkBox 复选框
     */
    void connectLoopCheckBox(int index, QCheckBox* checkBox);

    /**
     * @brief 打开脚本编辑器
     * @param scriptIndex 脚本编号 (1-6)
     */
    void openScriptEditor(int scriptIndex);

    /**
     * @brief 初始化脚本执行信号连接
     */
    void initScriptExecution();

    /**
     * @brief 获取当前脚本编辑器指针
     * @return 编辑器指针
     */
    ScriptEditor* currentEditor() const { return m_pCurrentScriptEditor; }

    /**
     * @brief 获取当前编辑的脚本索引
     * @return 脚本索引
     */
    int currentScriptIndex() const { return m_nCurrentScriptIndex; }

private:
    /**
     * @brief 执行指定索引的Lua脚本
     * @param index 脚本索引 (0-5)
     */
    void executeScript(int index);

    /**
     * @brief 设置循环使能状态
     * @param index 脚本索引 (0-5)
     * @param enable 是否启用
     */
    void setLoopEnable(int index, bool enable);

private:
    MainWorkFlow* m_workFlow;              ///< 工作流指针
    QWidget* m_parentWidget;               ///< 父窗口
    ScriptEditor* m_pCurrentScriptEditor;  ///< 当前脚本编辑器
    int m_nCurrentScriptIndex;             ///< 当前编辑的脚本索引
};

#endif // SCRIPTMANAGER_H
