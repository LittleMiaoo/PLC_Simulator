#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QMainWindow>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QCompleter>
#include <QMap>
#include <QStringList>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <functional>
#include <QTimer>
#include <QDialog>
#include <QLabel>

class LuaHighlighter;
class LuaScript;

// 脚本执行器接口，用于解耦 ScriptEditor 与具体执行实现
class IScriptRunner
{
public:
    virtual ~IScriptRunner() = default;

    // 异步执行脚本内容
    // scriptContent: 脚本代码
    // onFinished: 执行完成回调，参数为 (是否成功, 错误信息)
    virtual void RunScriptAsync(const QString& scriptContent,
                                std::function<void(bool success, const QString& errorMsg)> onFinished) = 0;
};

class ScriptEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr, IScriptRunner* pScriptRunner = nullptr);
    ~ScriptEditor();

    // 设置脚本执行器（依赖注入）
    void setScriptRunner(IScriptRunner* runner) { m_pScriptRunner = runner; }

    void setScriptName(const QString &name);
    void loadScript(const QString &content);
    QString getScriptContent() const;

private slots:
    void saveScript();
    void compileScript();
    void executeScript();
    void insertFunction(const QString &function);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void createMenus();
    //void createToolBar();
    void setupHighlighter();
    void updateFunctionMenu();
    //QStringList getRegisteredFunctions() const;
    
    QPlainTextEdit *editor;
    LuaHighlighter *highlighter;
    QString scriptFileName;
    QMap<QString, QString> functionTemplates;
    
    // 菜单动作
    QAction *saveAction;
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
    int m_nRunningSeconds = 0;           // 运行时间（秒）

    void showRunningDialog();            // 显示运行提示
    void hideRunningDialog();            // 隐藏运行提示
    void updateRunningTime();            // 更新运行时间
    void setEditorEnabled(bool enabled); // 设置编辑器启用状态
};

class LuaHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    LuaHighlighter(QTextDocument *parent = nullptr);

    void setCustomFunctions(const QStringList &functions);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // SCRIPTEDITOR_H