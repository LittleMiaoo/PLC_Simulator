#include "ScriptManager.h"
#include "ScriptEngineHost.h"
#include <QFileInfo>

ScriptManager::ScriptManager(ScriptEngineHost* host, QWidget* parent)
    : QObject(parent)
    , m_host(host)
    , m_parentWidget(parent)
    , m_pCurrentScriptEditor(nullptr)
    , m_nCurrentScriptIndex(-1)
{
    // 脚本完成 → 若是当前编辑器对应索引,转交编辑器收尾并提示(GUI 线程)
    if (m_host) {
        connect(m_host, &ScriptEngineHost::scriptFinished, this,
                [this](int index, bool ok, const QString& err) {
                    if (m_pCurrentScriptEditor && index == m_nCurrentScriptIndex)
                        m_pCurrentScriptEditor->onRunFinished(ok, err);
                });
    }
}

ScriptManager::~ScriptManager()
{
    // 编辑器会自动删除（设置了 WA_DeleteOnClose）
}

QString ScriptManager::luaPath(int index) const
{
    return QCoreApplication::applicationDirPath()
         + "/Config/LuaScript/LuaFile" + QString::number(index + 1) + ".lua";
}

void ScriptManager::runScript(int index)
{
    if (m_host == nullptr) return;

    const QString path = luaPath(index);
    if (!QFile::exists(path))
    {
        const QString msg = tr("脚本不存在: %1").arg(path);
        QMessageBox::critical(m_parentWidget, tr("Lua执行错误"), msg);
        emit logMessage(msg);
        return;
    }

    // 异步投递执行;运行结果(成功/失败)经 scriptFinished → scriptLog 反馈到日志
    m_host->runScript(index, path);
}

void ScriptManager::bindScriptRow(int index, QPushButton* execBtn, QPushButton* editBtn, QCheckBox* loopChk)
{
    if (execBtn)
        connect(execBtn, &QPushButton::clicked, this, [this, index]() {
            runScript(index);
        });

    if (editBtn)
        connect(editBtn, &QPushButton::clicked, this, [this, index]() {
            openScriptEditor(index);
        });

    if (loopChk)
        connect(loopChk, &QCheckBox::stateChanged, this, [this, index](int state) {
            if (m_host) m_host->setLoopValid(index, state == Qt::Checked);
        });
}

void ScriptManager::openScriptEditor(int index)
{
    if (m_host == nullptr) return;

    const QString scriptPath = luaPath(index);

    // 确保脚本目录存在(供编辑器保存时落盘;脚本文件本身在保存时才创建)
    QDir().mkpath(QFileInfo(scriptPath).absolutePath());

    if (!prepareForNewEditor(index)) return;   // 置前/取消:已处理,无需新建

    // 创建新的编辑器窗口，注入运行/检查 lambda
    ScriptEditor* pScriptEditor = new ScriptEditor(m_parentWidget);
    ScriptEngineHost* host = m_host;
    pScriptEditor->setRunFn(
        [host, index](const QString& content) {
            host->runScriptAsync(index, content);
        });
    pScriptEditor->setCheckFn(
        [host](const QString& script, QString& err) {
            return host->checkScript(script, err);
        });
    pScriptEditor->setFunctionDocs(m_host->functionDocs());
    pScriptEditor->setAttribute(Qt::WA_DeleteOnClose);
    pScriptEditor->setScriptName(scriptPath);

    // 加载文件内容(文件不存在则为空白)
    pScriptEditor->loadScript(readTextFile(scriptPath));

    // 保存编辑器指针
    m_pCurrentScriptEditor = pScriptEditor;
    m_nCurrentScriptIndex = index;

    // 连接关闭信号，清理指针
    connect(pScriptEditor, &QObject::destroyed, this, [this]() {
        m_pCurrentScriptEditor = nullptr;
        m_nCurrentScriptIndex = -1;
    });

    pScriptEditor->show();
}

bool ScriptManager::prepareForNewEditor(int index)
{
    if (m_pCurrentScriptEditor == nullptr) return true;

    // 同一脚本已打开:置前激活,无需新建
    if (m_nCurrentScriptIndex == index)
    {
        m_pCurrentScriptEditor->raise();
        m_pCurrentScriptEditor->activateWindow();
        return false;
    }

    // 切换到不同脚本:仅当有未保存更改时才提示保存/丢弃/取消
    if (m_pCurrentScriptEditor->isModified())
    {
        QMessageBox msgBox(m_parentWidget);
        msgBox.setWindowTitle(tr("切换脚本"));
        msgBox.setText(tr("当前脚本有未保存的更改,切换前是否保存?"));
        msgBox.setIcon(QMessageBox::Question);

        QPushButton* saveButton = msgBox.addButton(tr("保存"), QMessageBox::YesRole);
        msgBox.addButton(tr("不保存"), QMessageBox::NoRole);
        QPushButton* cancelButton = msgBox.addButton(tr("取消"), QMessageBox::RejectRole);

        msgBox.exec();

        QAbstractButton* clickedButton = msgBox.clickedButton();
        if (clickedButton == cancelButton) return false;

        if (clickedButton == saveButton)
            writeTextFile(luaPath(m_nCurrentScriptIndex), m_pCurrentScriptEditor->getScriptContent());
    }

    // 同步销毁当前编辑器:直接 delete(不调 close(),避免 WA_DeleteOnClose 的延迟删除与此处同步 delete 并存)。
    // delete 会同步触发 destroyed 信号,使清理槽在重新赋值前先把指针置空。
    delete m_pCurrentScriptEditor;
    m_pCurrentScriptEditor = nullptr;
    m_nCurrentScriptIndex = -1;
    return true;
}

QString ScriptManager::readTextFile(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    return in.readAll();
}

void ScriptManager::writeTextFile(const QString& path, const QString& text) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << text;
}
