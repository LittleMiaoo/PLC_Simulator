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

class LuaHighlighter;
class LuaScript;

class ScriptEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr,LuaScript* pLuaScript = nullptr);
    ~ScriptEditor();

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
    LuaScript* m_pLuaScript;
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