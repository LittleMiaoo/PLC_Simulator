#include "ScriptEditor.h"
#include "LuaScript.h"

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

ScriptEditor::ScriptEditor(QWidget *parent, IScriptRunner* pScriptRunner)
    : QMainWindow(parent)
    , editor(new CodeEditor(this))
    , highlighter(new LuaHighlighter(editor->document()))
    , m_pScriptRunner(pScriptRunner)
    , m_isModified(false)
    , m_savedContent("")
{
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

    // 连接文本修改信号
    connect(editor->document(), &QTextDocument::contentsChanged, this, &ScriptEditor::onTextChanged);

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
    fileMenu = menuBar()->addMenu(tr("&File"));

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    connect(saveAction, &QAction::triggered, this, &ScriptEditor::saveScript);
    fileMenu->addAction(saveAction);

    saveAsAction = new QAction(tr("Save &As..."), this);
    saveAsAction->setShortcut(tr("Ctrl+Alt+S"));
    connect(saveAsAction, &QAction::triggered, this, &ScriptEditor::saveScriptAs);
    fileMenu->addAction(saveAsAction);

    loadFromAction = new QAction(tr("&Load From..."), this);
    loadFromAction->setShortcut(tr("Ctrl+O"));
    connect(loadFromAction, &QAction::triggered, this, &ScriptEditor::loadScriptFrom);
    fileMenu->addAction(loadFromAction);

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
    if (LuaScript::CheckLuaScript(scriptContent, strError)) {
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

    // 执行前自动保存
    if (m_isModified && !scriptFileName.isEmpty()) {
        saveScript();
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
        m_pRunningLabel->setText(tr("Lua Running, UseTime: %1 s......").arg(m_nRunningSeconds));
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
    QString title = tr("Lua Script Editor");

    if (!scriptFileName.isEmpty()) {
        title += tr(" - %1").arg(scriptFileName);
    }

    if (m_isModified) {
        title += " *";
    }

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
    QString newFileName = QFileDialog::getSaveFileName(this, tr("Save Script As"),
                                                      defaultPath,
                                                      tr("Lua Scripts (*.lua)"));
    if (newFileName.isEmpty()) {
        return;  // 用户取消
    }

    // 保存到新文件
    QFile file(newFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot write file %1:\n%2.")
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

    QMessageBox::information(this, tr("Success"), tr("Script saved successfully."));
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
    QString loadFileName = QFileDialog::getOpenFileName(this, tr("Load Script From"),
                                                       defaultPath,
                                                       tr("Lua Scripts (*.lua)"));
    if (loadFileName.isEmpty()) {
        return;  // 用户取消
    }

    // 读取文件内容
    QFile file(loadFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot read file %1:\n%2.")
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
