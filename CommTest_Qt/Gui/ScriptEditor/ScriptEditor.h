#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <functional>
#include <QTimer>
#include <QDialog>
#include <QLabel>

#include "CodeEditor.h"
#include "LuaHighlighter.h"

class LuaScript;

// 脚本执行器接口,用于解耦 ScriptEditor 与具体执行实现
class IScriptRunner
{
public:
    virtual ~IScriptRunner() = default;

    // 异步执行脚本内容
    // scriptContent: 脚本代码
    // onFinished: 执行完成回调,参数为 (是否成功, 错误信息)
    virtual void RunScriptAsync(const QString& scriptContent,
                                std::function<void(bool success, const QString& errorMsg)> onFinished) = 0;
};

class ScriptEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr, IScriptRunner* pScriptRunner = nullptr);
    ~ScriptEditor();

    // 设置脚本执行器(依赖注入)
    void setScriptRunner(IScriptRunner* runner) { m_pScriptRunner = runner; }

    void setScriptName(const QString &name);
    void loadScript(const QString &content);
    QString getScriptContent() const;

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

private:
    void createMenus();
    void setupHighlighter();
    void updateFunctionMenu();
    void updateWindowTitle();  // 更新窗口标题(添加/移除星号)

    CodeEditor *editor;
    LuaHighlighter *highlighter;
    QString scriptFileName;
    QMap<QString, QString> functionTemplates;

    // 文件修改状态跟踪
    bool m_isModified;       // 文件是否被修改
    QString m_savedContent;  // 上次保存的内容

    // 菜单动作
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *loadFromAction;
    QAction *compileAction;
    QAction *executeAction;

    // 菜单
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *scriptMenu;
    QMenu *functionsMenu;

private:
    IScriptRunner* m_pScriptRunner;

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
