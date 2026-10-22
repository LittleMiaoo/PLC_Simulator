/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "ScriptEditor.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QMenuBar>
#include <QApplication>
#include <QDir>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QPushButton>
#include <QAbstractButton>
#include <QFileInfo>
#include <QEvent>

ScriptEditor::ScriptEditor(QWidget *parent)
    : QMainWindow(parent)
    , editor(new CodeEditor(this))
    , highlighter(new LuaHighlighter(editor->document()))
    , m_isModified(false)
    , m_savedContent("")
{
    setCentralWidget(editor);
    setWindowTitle(tr("Lua 脚本编辑器"));

    // 设置代码字体:优先现代等宽字体,逐级回退,确保代码对齐
    QFont font;
    font.setFamilies({"Cascadia Mono", "Consolas", "Courier New", "monospace"});
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(10);
    editor->setFont(font);  // CodeEditor 会在字体变化时按 IndentWidth 自动同步 Tab 视觉列宽

    createMenus();
    setupHighlighter();

    // 连接文本修改信号
    connect(editor->document(), &QTextDocument::contentsChanged, this, &ScriptEditor::onTextChanged);

    // 语言结构模板（与绑定函数同构,统一为 LuaFunctionDoc：name / snippet / description）
    m_langTemplates = langTemplates();

    updateFunctionMenu();

    resize(800, 600);
}

ScriptEditor::~ScriptEditor()
{
}

void ScriptEditor::setScriptName(const QString &name)
{
    scriptFileName = name;
    updateWindowTitle();
}

void ScriptEditor::loadScript(const QString &content)
{
    editor->setPlainText(content);
    m_savedContent = content;
    m_isModified = false;
    updateWindowTitle();
}

QString ScriptEditor::getScriptContent() const
{
    return editor->toPlainText();
}

void ScriptEditor::createMenus()
{
    // 文件菜单
    fileMenu = menuBar()->addMenu(tr("文件(&F)"));

    saveAction = new QAction(tr("保存(&S)"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    connect(saveAction, &QAction::triggered, this, &ScriptEditor::saveScript);
    fileMenu->addAction(saveAction);

    saveAsAction = new QAction(tr("另存为(&A)..."), this);
    saveAsAction->setShortcut(tr("Ctrl+Alt+S"));
    connect(saveAsAction, &QAction::triggered, this, &ScriptEditor::saveScriptAs);
    fileMenu->addAction(saveAsAction);

    loadFromAction = new QAction(tr("从文件加载(&L)..."), this);
    loadFromAction->setShortcut(tr("Ctrl+O"));
    connect(loadFromAction, &QAction::triggered, this, &ScriptEditor::loadScriptFrom);
    fileMenu->addAction(loadFromAction);

    fileMenu->addSeparator();

    exitAction = new QAction(tr("退出(&X)"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    // 编辑菜单
    editMenu = menuBar()->addMenu(tr("编辑(&E)"));

    functionsMenu = editMenu->addMenu(tr("插入函数(&I)"));

    // 脚本菜单
    scriptMenu = menuBar()->addMenu(tr("脚本(&S)"));

    compileAction = new QAction(tr("编译脚本(&C)"), this);
    compileAction->setShortcut(tr("F7"));
    connect(compileAction, &QAction::triggered, this, &ScriptEditor::compileScript);
    scriptMenu->addAction(compileAction);

    scriptMenu->addSeparator();

    executeAction = new QAction(tr("执行脚本(&E)"), this);
    executeAction->setShortcut(tr("F5"));
    connect(executeAction, &QAction::triggered, this, &ScriptEditor::executeScript);
    scriptMenu->addAction(executeAction);
}

void ScriptEditor::retranslateMenus()
{
    setWindowTitle(tr("Lua 脚本编辑器"));
    updateWindowTitle();

    if (fileMenu)
        fileMenu->setTitle(tr("文件(&F)"));
    if (saveAction)
        saveAction->setText(tr("保存(&S)"));
    if (saveAsAction)
        saveAsAction->setText(tr("另存为(&A)..."));
    if (loadFromAction)
        loadFromAction->setText(tr("从文件加载(&L)..."));
    if (exitAction)
        exitAction->setText(tr("退出(&X)"));
    if (editMenu)
        editMenu->setTitle(tr("编辑(&E)"));
    if (functionsMenu)
        functionsMenu->setTitle(tr("插入函数(&I)"));
    if (scriptMenu)
        scriptMenu->setTitle(tr("脚本(&S)"));
    if (compileAction)
        compileAction->setText(tr("编译脚本(&C)"));
    if (executeAction)
        executeAction->setText(tr("执行脚本(&E)"));

    m_langTemplates = langTemplates();
    updateFunctionMenu();
}

QList<LuaFunctionDoc> ScriptEditor::langTemplates()
{
    return {
        {"if",             "if (condition1) then\n    \nend", tr("if 条件分支")},
        {"while",          "while (condition1) do\n    \nend", tr("while 循环")},
        {"for",            "for i = 1, 10 do\n    \nend", tr("for 循环")},
        {"if-elseif-else", "if (condition1) then\n    \n"
                           "elseif (condition2) then\n    \n"
                           "else\n    \nend", tr("if-elseif-else 多分支")},
    };
}

void ScriptEditor::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateMenus();
    QMainWindow::changeEvent(event);
}

void ScriptEditor::setupHighlighter()
{
    // 高亮自定义函数名由 m_functionDocs 提供（注入后通过 setFunctionDocs 刷新）
    // 此处初始化为空，待 setFunctionDocs 调用后更新
    highlighter->setCustomFunctions(QStringList());
}

void ScriptEditor::setFunctionDocs(const QList<LuaFunctionDoc>& docs)
{
    m_functionDocs = docs;

    // 更新函数菜单
    updateFunctionMenu();

    // 更新高亮器：将绑定函数名提取为自定义函数列表
    QStringList names;
    for (const LuaFunctionDoc& doc : m_functionDocs) {
        names.append(doc.name);
    }
    highlighter->setCustomFunctions(names);
}

void ScriptEditor::updateFunctionMenu()
{
    // 清除现有动作
    functionsMenu->clear();
    functionsMenu->setToolTipsVisible(true);  // 让菜单项 tooltip 可见(展示功能描述)

    addFunctionMenuGroup(m_langTemplates);    // 条件控制(语言结构 if/while/for/...) 置顶

    // 若两组都有内容，加分隔线
    if (!m_langTemplates.isEmpty() && !m_functionDocs.isEmpty()) {
        functionsMenu->addSeparator();
    }

    addFunctionMenuGroup(m_functionDocs);     // 绑定+内建函数(sleep/IsLoopValid/寄存器/平台)
}

void ScriptEditor::addFunctionMenuGroup(const QList<LuaFunctionDoc>& docs)
{
    for (const LuaFunctionDoc& doc : docs) {
        QAction *action = new QAction(doc.name, this);
        action->setToolTip(doc.description);    // 悬停显示功能描述,不插入
        action->setStatusTip(doc.description);
        connect(action, &QAction::triggered, this, [this, name = doc.name] {
            insertFunction(name);
        });
        functionsMenu->addAction(action);
    }
}

void ScriptEditor::saveScript()
{
    if (scriptFileName.isEmpty()) {
        scriptFileName = QFileDialog::getSaveFileName(this, tr("保存脚本"),
                                                     QDir::homePath(),
                                                     tr("Lua 脚本 (*.lua);;所有文件 (*)"));
        if (scriptFileName.isEmpty())
            return;
    }

    QFile file(scriptFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"),
                             tr("无法写入文件 %1:\n%2。")
                             .arg(scriptFileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    QString content = editor->toPlainText();
    out << content;
    file.close();

    // 保存成功后,重置修改状态
    m_savedContent = content;
    m_isModified = false;
    updateWindowTitle();

   // QMessageBox::information(this, tr("Success"), tr("Script saved successfully."));
}

void ScriptEditor::compileScript()
{
    // 编译前自动保存
    if (m_isModified && !scriptFileName.isEmpty()) {
        saveScript();
    }

    QString strError;
    QString scriptContent = editor->toPlainText();
    if (!m_checkFn) {
        QMessageBox::warning(this, tr("编译"), tr("语法检查器不可用。"));
        return;
    }
    if (m_checkFn(scriptContent, strError)) {
        QMessageBox::information(this, tr("编译"), tr("脚本编译成功。"));
    } else {
        QMessageBox::critical(this, tr("编译错误"), strError);
    }
}

void ScriptEditor::executeScript()
{
    if (!m_runFn) {
        QMessageBox::warning(this, tr("错误"), tr("未配置脚本运行器。"));
        return;
    }

    if (m_bExecuting) {
        return; // 防止重复执行
    }

    // 执行前自动保存
    if (m_isModified && !scriptFileName.isEmpty()) {
        saveScript();
    }

    // 先编译检查
    QString scriptContent = editor->toPlainText();
    QString strError;
    if (m_checkFn && !m_checkFn(scriptContent, strError)) {
        QMessageBox::critical(this, tr("编译错误"), strError);
        return;
    }

    // 禁用界面并显示运行提示
    m_bExecuting = true;
    setEditorEnabled(false);
    showRunningDialog();

    // 异步执行;完成结果经 ScriptEngineHost::scriptFinished → ScriptManager → onRunFinished 回流
    m_runFn(scriptContent);
}

void ScriptEditor::onRunFinished(bool ok, const QString& err)
{
    // 收尾运行态:隐藏提示、恢复界面、清执行标志(与启动时 showRunningDialog/setEditorEnabled 对应)
    hideRunningDialog();
    setEditorEnabled(true);
    m_bExecuting = false;

    if (ok)
        QMessageBox::information(this, tr("执行"), tr("脚本执行成功。"));
    else
        QMessageBox::critical(this, tr("执行错误"), err);
}

void ScriptEditor::showRunningDialog()
{
    m_nRunningSeconds = 0;

    // 创建不可关闭的对话框
    m_pRunningDialog = new QDialog(this);
    m_pRunningDialog->setWindowTitle(tr("脚本运行中"));
    m_pRunningDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    m_pRunningDialog->setModal(true);
    m_pRunningDialog->setFixedSize(280, 80);

    // 创建标签
    m_pRunningLabel = new QLabel(tr("Lua 运行中，用时: 0 秒......"), m_pRunningDialog);
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
        m_pRunningLabel->setText(tr("Lua 运行中，用时: %1 秒......").arg(m_nRunningSeconds));
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
    // 按名查 snippet：先绑定函数,再语言结构模板(同构,统一查找)
    QString templateStr;
    for (const LuaFunctionDoc& doc : m_functionDocs) {
        if (doc.name == function) { templateStr = doc.snippet; break; }
    }
    if (templateStr.isEmpty()) {
        for (const LuaFunctionDoc& doc : m_langTemplates) {
            if (doc.name == function) { templateStr = doc.snippet; break; }
        }
    }

    if (templateStr.isEmpty()) return;

    QTextCursor cursor = editor->textCursor();

    // 检查当前行是否有内容
    QTextBlock currentBlock = cursor.block();
    bool lineHasContent = !currentBlock.text().trimmed().isEmpty();

    // 如果当前行有内容,先换行
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

    // 检查是否有未保存的修改
    if (m_isModified) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("文件未保存"));
        msgBox.setText(tr("当前脚本已修改。是否保存到本地?"));
        msgBox.setIcon(QMessageBox::Question);

        QPushButton *saveButton = msgBox.addButton(tr("保存"), QMessageBox::AcceptRole);
        QPushButton *discardButton = msgBox.addButton(tr("不保存"), QMessageBox::DestructiveRole);
        QPushButton *cancelButton = msgBox.addButton(tr("取消"), QMessageBox::RejectRole);

        msgBox.exec();

        QAbstractButton *clicked = msgBox.clickedButton();
        if (clicked == static_cast<QAbstractButton*>(saveButton)) {
            saveScript();
            // 如果保存成功(m_isModified被重置),则允许关闭
            if (m_isModified) {
                event->ignore();
                return;
            }
        } else if (clicked == static_cast<QAbstractButton*>(cancelButton)) {
            event->ignore();
            return;
        }
        // discardButton: 直接关闭,不保存
    }

    QMainWindow::closeEvent(event);
}

void ScriptEditor::onTextChanged()
{
    QString currentContent = editor->toPlainText();

    // 检查当前内容是否与保存的内容相同
    bool isNowModified = (currentContent != m_savedContent);

    // 只有当修改状态发生变化时才更新
    if (isNowModified != m_isModified) {
        m_isModified = isNowModified;
        updateWindowTitle();
    }
}

void ScriptEditor::updateWindowTitle()
{
    const QString appName = tr("Lua 脚本编辑器");
    QString title;

    if (!scriptFileName.isEmpty()) {
        // 文件名在前、应用名在后(任务栏/Alt-Tab 截断时文件名优先可见);
        // 标题仅显示文件名,完整路径仍保存在 scriptFileName 中供文件读写。
        // [*] 紧跟文件名,由 Qt 据 windowModified 自动显示/隐藏修改标记(星号)。
        title = tr("%1[*] - %2").arg(QFileInfo(scriptFileName).fileName(), appName);
    } else {
        title = appName + "[*]";
    }

    setWindowModified(m_isModified);
    setWindowTitle(title);
}

void ScriptEditor::saveScriptAs()
{
    // 获取当前文件所在目录作为默认路径
    QString defaultPath = QDir::homePath();
    if (!scriptFileName.isEmpty()) {
        QFileInfo fileInfo(scriptFileName);
        defaultPath = fileInfo.absolutePath();
    }

    // 弹出文件保存对话框
    QString newFileName = QFileDialog::getSaveFileName(this, tr("另存为脚本"),
                                                      defaultPath,
                                                      tr("Lua 脚本 (*.lua)"));
    if (newFileName.isEmpty()) {
        return;  // 用户取消
    }

    // 保存到新文件
    QFile file(newFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"),
                             tr("无法写入文件 %1:\n%2。")
                             .arg(newFileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    QString content = editor->toPlainText();
    out << content;
    file.close();

    // 更新当前文件路径
    scriptFileName = newFileName;
    m_savedContent = content;
    m_isModified = false;
    updateWindowTitle();

    QMessageBox::information(this, tr("成功"), tr("脚本保存成功。"));
}

void ScriptEditor::loadScriptFrom()
{
    // 获取当前文件所在目录作为默认路径
    QString defaultPath = QDir::homePath();
    if (!scriptFileName.isEmpty()) {
        QFileInfo fileInfo(scriptFileName);
        defaultPath = fileInfo.absolutePath();
    }

    // 弹出文件选择对话框
    QString loadFileName = QFileDialog::getOpenFileName(this, tr("从文件加载脚本"),
                                                       defaultPath,
                                                       tr("Lua 脚本 (*.lua)"));
    if (loadFileName.isEmpty()) {
        return;  // 用户取消
    }

    // 读取文件内容
    QFile file(loadFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"),
                             tr("无法读取文件 %1:\n%2。")
                             .arg(loadFileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();

    // 加载内容到编辑器,但不修改当前文件路径
    editor->setPlainText(content);
    // 注意:不更新 scriptFileName,保持原文件路径
    // 标记为已修改,因为内容与当前文件不同
    m_isModified = true;
    updateWindowTitle();
}
