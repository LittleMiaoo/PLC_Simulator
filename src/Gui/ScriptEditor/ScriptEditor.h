/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <functional>
#include <QTimer>
#include <QDialog>
#include <QLabel>
#include <QEvent>

#include "CodeEditor.h"
#include "LuaHighlighter.h"
#include "ILuaBinding.h"

class ScriptEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);
    ~ScriptEditor();

    // 注入运行函数(std::function 形式，由 ScriptManager 通过 lambda 提供);完成结果经 onRunFinished 回流
    using RunFn   = std::function<void(const QString& content)>;
    using CheckFn = std::function<bool(const QString& script, QString& err)>;
    void setRunFn(RunFn fn)     { m_runFn = std::move(fn); }
    void setCheckFn(CheckFn fn) { m_checkFn = std::move(fn); }

    // 注入绑定函数文档列表（供函数菜单与语法高亮使用）
    void setFunctionDocs(const QList<LuaFunctionDoc>& docs);


    void setScriptName(const QString &name);
    void loadScript(const QString &content);
    QString getScriptContent() const;

    // 是否有未保存的更改(内容 ≠ 上次加载/保存)
    bool isModified() const { return m_isModified; }

    // 脚本运行完成时由 ScriptManager 在 GUI 线程调用:收尾运行态并提示结果
    void onRunFinished(bool ok, const QString& err);

private slots:
    void saveScript();
    void saveScriptAs();   // 另存为
    void loadScriptFrom(); // 从文件加载
    void compileScript();
    void executeScript();
    void insertFunction(const QString &function);
    void onTextChanged();  // 文本修改时的槽函数

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent* event) override;

private:
    void createMenus();
    void retranslateMenus();
    static QList<LuaFunctionDoc> langTemplates();   // 语言结构模板(按当前语言翻译说明),构造与语言切换共用
    void setupHighlighter();
    void updateFunctionMenu();
    void addFunctionMenuGroup(const QList<LuaFunctionDoc>& docs);  // 向函数菜单添加一组(绑定/语言结构)
    void updateWindowTitle();  // 更新窗口标题(添加/移除星号)

    CodeEditor *editor;
    LuaHighlighter *highlighter;
    QString scriptFileName;
    QList<LuaFunctionDoc> m_langTemplates;  // 语言结构模板(if/while/for/...),与绑定函数同构

    // 文件修改状态跟踪
    bool m_isModified;       // 文件是否被修改
    QString m_savedContent;  // 上次保存的内容

    // 菜单动作
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *loadFromAction;
    QAction *exitAction;
    QAction *compileAction;
    QAction *executeAction;

    // 菜单
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *scriptMenu;
    QMenu *functionsMenu;

private:
    RunFn   m_runFn;                        // 注入的脚本运行函数
    CheckFn m_checkFn;                      // 注入的语法检查函数
    QList<LuaFunctionDoc> m_functionDocs;  // 注入的绑定函数文档列表

    // 执行状态相关
    bool m_bExecuting = false;           // 是否正在执行脚本
    QDialog* m_pRunningDialog = nullptr; // 运行提示对话框
    QLabel* m_pRunningLabel = nullptr;   // 运行提示标签
    QTimer* m_pRunningTimer = nullptr;   // 计时器
    int m_nRunningSeconds = 0;           // 运行时间(秒)

    void showRunningDialog();            // 显示运行提示
    void hideRunningDialog();            // 隐藏运行提示
    void updateRunningTime();            // 更新运行时间
    void setEditorEnabled(bool enabled); // 设置编辑器启用状态
};

#endif // SCRIPTEDITOR_H
