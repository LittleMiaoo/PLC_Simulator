#include "ScriptEditor.h"
#include "LuaScript.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>
#include <QDir>
#include <QCloseEvent>
#include <QDateTime>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QActionGroup>
#include <QInputDialog>

// 这里假设您有一个LuaScript类
// 如果实际类名不同，请相应调整
// class LuaScript {
// public:


ScriptEditor::ScriptEditor(QWidget *parent, IScriptRunner* pScriptRunner)
    : QMainWindow(parent)
    , editor(new QPlainTextEdit(this))
    , highlighter(new LuaHighlighter(editor->document()))
    , m_pScriptRunner(pScriptRunner)
{
//     if (pLuaScript != nullptr)
//     {
//         m_pLuaScript = pLuaScript;
//     }
    
    setCentralWidget(editor);
    setWindowTitle(tr("Lua Script Editor"));
    
    // 设置字体
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    editor->setFont(font);
    
    createMenus();
    setupHighlighter();

    
    // 设置初始函数模板
    functionTemplates["SetInt16"] = "SetInt16(\"D100\", 123) -- 设置D100为123,单字";
    functionTemplates["SetInt32"] = "SetInt32(\"D100\", 123)    -- 设置D100为123,双字";
    functionTemplates["SetFloat"] = "SetFloat(\"D100\", 123.45) -- 设置D100为123.45,浮点数";
    functionTemplates["SetDouble"] = "SetDouble(\"D100\", 123.45) -- 设置D100为123.45,双精度浮点数";
    functionTemplates["SetString"] = "SetString(\"D100\", \"AB\") -- 设置D100为字符串AB";

    functionTemplates["GetInt16"] = "GetInt16(\"D100\") -- 获取D100的值,单字";
    functionTemplates["GetInt32"] = "GetInt32(\"D100\")   -- 获取D100的值,双字";
    functionTemplates["GetFloat"] = "GetFloat(\"D100\") -- 获取D100的值,浮点数";
    functionTemplates["GetDouble"] = "GetDouble(\"D100\") -- 获取D100的值,双精度浮点数";
    functionTemplates["GetString"] = "GetString(\"D100\") -- 获取D100的值,字符串";

    functionTemplates["MoveAbsInt32"] = "MoveAbsInt32(\"D100\", \"D102\", \"D104\") --根据指定寄存器绝对移动,双字";
    functionTemplates["MoveAbsFloat"] = "MoveAbsFloat(\"D100\", \"D102\", \"D104\") --根据指定寄存器绝对移动,浮点数";
    functionTemplates["MoveRelativeInt32"] = "MoveRelativeInt32(\"D100\", \"D102\", \"D104\") --根据指定寄存器相对移动,双字";
    functionTemplates["MoveRelativeFloat"] = "MoveRelativeFloat(\"D100\", \"D102\", \"D104\") --根据指定寄存器相对移动,浮点数";
    functionTemplates["WriteCurrentPosInt32"] = "WriteCurrentPosInt32(\"D100\", \"D102\", \"D104\") --写入当前位置,双字";
    functionTemplates["WriteCurrentPosFloat"] = "WriteCurrentPosFloat(\"D100\", \"D102\", \"D104\") --写入当前位置,浮点数";

    functionTemplates["IsLoopValid"] = "IsLoopValid() -- 获取循环是否有效";
    functionTemplates["sleep"] = "sleep(500) -- 睡眠500毫秒";
    //lua判定语法
    functionTemplates["if"] = "if (condition1) then\n    -- 条件condition1为真时执行的代码\nend";
    functionTemplates["while"] = "while (condition1) do\n    -- 条件condition1为真时执行的代码\nend";
    functionTemplates["for"] = "for i = 1, 10 do\n    -- 循环体代码\nend";
    functionTemplates["if-elseif-else"] = "if (condition1) then\n    -- 条件condition1为真时执行的代码\n" \
                                        "elseif (condition2) then\n    -- 条件condition2为真时执行的代码\n" \
                                        "else\n    -- 所有条件均不为真时执行的代码\nend";
    
    updateFunctionMenu();

    resize(800, 600);
}

ScriptEditor::~ScriptEditor()
{
}

void ScriptEditor::setScriptName(const QString &name)
{
    scriptFileName = name;
    setWindowTitle(tr("Lua Script Editor - %1").arg(name));
}

void ScriptEditor::loadScript(const QString &content)
{
    editor->setPlainText(content);
}

QString ScriptEditor::getScriptContent() const
{
    return editor->toPlainText();
}

void ScriptEditor::createMenus()
{
    // 文件菜单
    fileMenu = menuBar()->addMenu(tr("&File"));
    
    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    connect(saveAction, &QAction::triggered, this, &ScriptEditor::saveScript);
    fileMenu->addAction(saveAction);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = new QAction(tr("&Exit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    
    // 编辑菜单
    editMenu = menuBar()->addMenu(tr("&Edit"));
    
    functionsMenu = editMenu->addMenu(tr("&Insert Function"));
    
    // 脚本菜单
    scriptMenu = menuBar()->addMenu(tr("&Script"));
    
    compileAction = new QAction(tr("&Compile Script"), this);
    compileAction->setShortcut(tr("F7"));
    connect(compileAction, &QAction::triggered, this, &ScriptEditor::compileScript);
    scriptMenu->addAction(compileAction);
    
    scriptMenu->addSeparator();
    
    executeAction = new QAction(tr("&Execute Script"), this);
    executeAction->setShortcut(tr("F5"));
    connect(executeAction, &QAction::triggered, this, &ScriptEditor::executeScript);
    scriptMenu->addAction(executeAction);
}

void ScriptEditor::setupHighlighter()
{
    // 设置自定义函数列表
    QStringList customFunctions = LuaScript::getRegisteredFunctions();
    highlighter->setCustomFunctions(customFunctions);
}

void ScriptEditor::updateFunctionMenu()
{
    // 清除现有动作
    functionsMenu->clear();
    
    // 获取注册的函数并添加到菜单
    // QStringList functions = LuaScript::getRegisteredFunctions();
    QStringList functions;
    for (auto it = functionTemplates.begin(); it != functionTemplates.end(); ++it) {
        functions.append(it.key());
    }

    for (const QString &function : functions) {
        QAction *action = new QAction(function, this);

        connect(action, &QAction::triggered, this, [=] {
            insertFunction(function);
        });

        functionsMenu->addAction(action);
    }
}

void ScriptEditor::saveScript()
{
    if (scriptFileName.isEmpty()) {
        scriptFileName = QFileDialog::getSaveFileName(this, tr("Save Script"), 
                                                     QDir::homePath(), 
                                                     tr("Lua Scripts (*.lua);;All Files (*)"));
        if (scriptFileName.isEmpty())
            return;
            
        setWindowTitle(tr("Lua Script Editor - %1").arg(scriptFileName));
    }
    
    QFile file(scriptFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), 
                             tr("Cannot write file %1:\n%2.")
                             .arg(scriptFileName)
                             .arg(file.errorString()));
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << editor->toPlainText();
    file.close();
    
    QMessageBox::information(this, tr("Success"), tr("Script saved successfully."));
}

void ScriptEditor::compileScript()
{
    QString strError ;
    QString scriptContent = editor->toPlainText();
    if (LuaScript::CheckLuaScript(scriptContent,strError)) {
        QMessageBox::information(this, tr("Compile"), tr("Script compiled successfully."));
    } else {
        QMessageBox::critical(this, tr("Compile Error"), strError);
    }
}

void ScriptEditor::executeScript()
{
    if (!m_pScriptRunner) {
        QMessageBox::warning(this, tr("Error"), tr("Script runner not configured."));
        return;
    }

    if (m_bExecuting) {
        return; // 防止重复执行
    }

    // 先编译检查
    QString scriptContent = editor->toPlainText();
    QString strError;
    if (!LuaScript::CheckLuaScript(scriptContent, strError)) {
        QMessageBox::critical(this, tr("Compile Error"), strError);
        return;
    }

    // 禁用界面并显示运行提示
    m_bExecuting = true;
    setEditorEnabled(false);
    showRunningDialog();

    // 异步执行脚本
    m_pScriptRunner->RunScriptAsync(scriptContent,
        [this](bool success, const QString& errorMsg) {
            // 回调在主线程执行
            QMetaObject::invokeMethod(this, [this, success, errorMsg]() {
                // 隐藏运行提示并恢复界面
                hideRunningDialog();
                setEditorEnabled(true);
                m_bExecuting = false;

                if (success) {
                    QMessageBox::information(this, tr("Execute"), tr("Script executed successfully."));
                } else {
                    QMessageBox::critical(this, tr("Execution Error"), errorMsg);
                }
            }, Qt::QueuedConnection);
        });
}

void ScriptEditor::showRunningDialog()
{
    m_nRunningSeconds = 0;

    // 创建不可关闭的对话框
    m_pRunningDialog = new QDialog(this);
    m_pRunningDialog->setWindowTitle(tr("Script Running"));
    m_pRunningDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    m_pRunningDialog->setModal(true);
    m_pRunningDialog->setFixedSize(280, 80);

    // 创建标签
    m_pRunningLabel = new QLabel(tr("Lua is Running, Use-Time: 0 s......"), m_pRunningDialog);
    m_pRunningLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layout = new QVBoxLayout(m_pRunningDialog);
    layout->addWidget(m_pRunningLabel);
    m_pRunningDialog->setLayout(layout);

    // 创建计时器
    m_pRunningTimer = new QTimer(this);
    connect(m_pRunningTimer, &QTimer::timeout, this, &ScriptEditor::updateRunningTime);
    m_pRunningTimer->start(1000); // 每秒更新

    m_pRunningDialog->show();
}

void ScriptEditor::hideRunningDialog()
{
    // 停止计时器
    if (m_pRunningTimer) {
        m_pRunningTimer->stop();
        delete m_pRunningTimer;
        m_pRunningTimer = nullptr;
    }

    // 关闭并删除对话框
    if (m_pRunningDialog) {
        m_pRunningDialog->close();
        delete m_pRunningDialog;
        m_pRunningDialog = nullptr;
        m_pRunningLabel = nullptr; // 已随对话框删除
    }
}

void ScriptEditor::updateRunningTime()
{
    m_nRunningSeconds++;
    if (m_pRunningLabel) {
        m_pRunningLabel->setText(tr("Lua Running, UseTime: %1 S......").arg(m_nRunningSeconds));
    }
}

void ScriptEditor::setEditorEnabled(bool enabled)
{
    // 禁用/启用编辑器
    editor->setEnabled(enabled);

    // 禁用/启用菜单栏
    menuBar()->setEnabled(enabled);
}

void ScriptEditor::insertFunction(const QString &function)
{
    QString templateStr = functionTemplates[function];

    if (templateStr == "") return;
    
    QTextCursor cursor = editor->textCursor();
    
    // 检查当前行是否有内容
    QTextBlock currentBlock = cursor.block();
    bool lineHasContent = !currentBlock.text().trimmed().isEmpty();
    
    // 如果当前行有内容，先换行
    if (lineHasContent) {
        cursor.insertText("\n");
    }
    
    // 插入函数
    cursor.insertText(templateStr);
    
    // 插入完成后换行
    cursor.insertText("\n");
    
    // 设置光标位置
    editor->setTextCursor(cursor);
}

void ScriptEditor::closeEvent(QCloseEvent *event)
{
    // 脚本执行中不允许关闭窗口
    if (m_bExecuting) {
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}

// LuaHighlighter实现
LuaHighlighter::LuaHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    //调整高亮规则顺序
    //1.关键字
    //2.字符串(关键字字符串'"and"'也应该是字符颜色)
    //3.注释(所有内容都可以被注释、显示注释颜色)
    //① '--and':显示绿色
    //②'--"and"':绿色
    //③'--SetInt16':绿色

    HighlightingRule rule;

    // 关键字格式（蓝色）
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    
    QStringList keywordPatterns;
    keywordPatterns << "\\band\\b" << "\\bbreak\\b" << "\\bdo\\b" << "\\belse\\b"
                    << "\\belseif\\b" << "\\bend\\b" << "\\bfalse\\b" << "\\bfor\\b"
                    << "\\bfunction\\b" << "\\bif\\b" << "\\bin\\b" << "\\blocal\\b"
                    << "\\bnil\\b" << "\\bnot\\b" << "\\bor\\b" << "\\brepeat\\b"
                    << "\\breturn\\b" << "\\bthen\\b" << "\\btrue\\b" << "\\buntil\\b"
                    << "\\bwhile\\b";

    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

     // 字符串格式
    quotationFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    
    rule.pattern = QRegularExpression("'.*'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 函数格式（橙色）
    functionFormat.setForeground(QColor(255, 165, 0)); // 橙色
    functionFormat.setFontWeight(QFont::Bold);

    // 单行注释格式
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("--[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 多行注释格式
    commentStartExpression = QRegularExpression("--\\[\\[");
    commentEndExpression = QRegularExpression("\\]\\]");
}

void LuaHighlighter::setCustomFunctions(const QStringList &functions)
{
    // 更新自定义函数的高亮规则
    for (const QString &function : functions) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + function + "\\b");
        rule.format = functionFormat;
        //highlightingRules.append(rule);
        highlightingRules.insert(0,rule); // 将自定义函数规则插入到规则列表的开头
    }
}

void LuaHighlighter::highlightBlock(const QString &text)
{
    // for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
    //     QRegExp expression(rule.pattern);
    //     int index = expression.indexIn(text);
    //     while (index >= 0) {
    //         int length = expression.matchedLength();
    //         setFormat(index, length, rule.format);
    //         index = expression.indexIn(text, index + length);
    //     }
    // }
     for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    
    // 处理多行注释
    setCurrentBlockState(0);

    // int startIndex = 0;
    // if (previousBlockState() != 1)
    //     startIndex = commentStartExpression.indexIn(text);
     int startIndex = 0;
    if (previousBlockState() != 1) {
        QRegularExpressionMatch startMatch = commentStartExpression.match(text);
        if (startMatch.hasMatch()) {
            startIndex = startMatch.capturedStart();
        } else {
            startIndex = -1;
        }
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = commentEndExpression.match(text, startIndex);
        int endIndex = -1;
        int commentLength = 0;
        
        if (endMatch.hasMatch()) {
            endIndex = endMatch.capturedStart();
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        } else {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        
        setFormat(startIndex, commentLength, singleLineCommentFormat);
        
        // 寻找下一个多行注释开始
        QRegularExpressionMatch nextStartMatch = commentStartExpression.match(text, startIndex + commentLength);
        if (nextStartMatch.hasMatch()) {
            startIndex = nextStartMatch.capturedStart();
        } else {
            startIndex = -1;
        }
    }
}