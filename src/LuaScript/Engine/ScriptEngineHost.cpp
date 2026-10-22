/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */
#include "ScriptEngineHost.h"

#include "LuaEngine.h"
#include "LuaStaticCheck.h"
#include "Core/RegisterStore.h"

#include <QSet>
#include <QThreadPool>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QRunnable>
#include <QElapsedTimer>
#include <QDebug>

ScriptEngineHost::ScriptEngineHost(RegisterStore* store, int engineCount, QObject* parent)
    : QObject(parent), m_store(store), m_engineCount(engineCount)
{
    m_threadPool = new QThreadPool(this);
    m_threadPool->setMaxThreadCount(QThread::idealThreadCount());

    // 任一脚本完成后:失败统一转发到脚本日志(主窗口监听 scriptLog);编辑器结果另由其自身收尾。
    connect(this, &ScriptEngineHost::scriptFinished, this, [this](int, bool ok, const QString& err){
        if (!ok) emit scriptLog(tr("Lua执行失败:%1").arg(err));
    });

    // 绑定一律由构造后的 installModule() 注入(其会装入所有已建引擎);此处仅创建引擎与各自的互斥锁。
    m_engines.resize(m_engineCount);
    m_mutexes.resize(m_engineCount);
    for (int i = 0; i < m_engineCount; ++i) {
        m_engines[i] = std::unique_ptr<LuaEngine>(LuaEngine::InitialEngine());
        m_mutexes[i] = std::make_unique<QMutex>();
    }
}

ScriptEngineHost::~ScriptEngineHost()
{
    // 先令循环脚本尽快退出循环
    for (auto& e : m_engines)
        if (e) e->SetLoopValid(false);

    if (m_threadPool) {
        QElapsedTimer drainTimer; drainTimer.start();
        bool warned = false;
        while (!m_threadPool->waitForDone(50)) {
            if (!warned && drainTimer.elapsed() > 3000) {
                qWarning() << "ScriptEngineHost: 池任务 drain 超过 3s,可能有脚本未检查 IsLoopValid 而无法退出";
                warned = true;
            }
        }
    }

    m_engines.clear();   // 引擎(持绑定闭包)先于绑定释放
    m_modules.clear();
}

void ScriptEngineHost::runOnPool(int index, std::function<bool(LuaEngine*, QString&)> exec)
{
    // 唯一执行咽喉:两条运行入口(面板/编辑器)都经此。started 在校验前发,与 finished 严格配对。
    emit scriptStarted(index);
    if (index < 0 || index >= m_engineCount || !m_engines[index]) {
        emit scriptFinished(index, false, QString("LuaEngine instance not found"));
        return;
    }
    class Task : public QRunnable {
    public:
        LuaEngine* lua = nullptr; QMutex* mutex = nullptr;
        std::function<bool(LuaEngine*, QString&)> exec;
        std::function<void(bool, const QString&)> done;
        void run() override {
            QMutexLocker locker(mutex);
            QString err;
            const bool ok = exec(lua, err);
            if (done) done(ok, err);
        }
    };
    Task* t = new Task;
    t->lua   = m_engines[index].get();
    t->mutex = m_mutexes[index].get();
    t->exec  = std::move(exec);
    // 完成回调在工作线程触发,只发 scriptFinished(跨线程自动排队);具体处理由各接收者在其线程进行。
    t->done  = [this, index](bool ok, const QString& err){ emit scriptFinished(index, ok, err); };
    t->setAutoDelete(true);
    m_threadPool->start(t);
}

void ScriptEngineHost::runScript(int index, const QString& luaFile)
{
    runOnPool(index,
        [luaFile](LuaEngine* e, QString& err){ return e->RunLuaScript(luaFile, err); });
}

void ScriptEngineHost::runScriptAsync(int index, const QString& content)
{
    runOnPool(index,
        [content](LuaEngine* e, QString& err){ return e->RunLuaScriptWithEditor(content, err); });
}

bool ScriptEngineHost::checkScript(const QString& script, QString& errorMsg) const
{
    QSet<QString> known;
    for (const LuaFunctionDoc& d : functionDocs()) known.insert(d.name);
    return LuaStaticCheck::run(script, known, errorMsg);
}

LuaEngine* ScriptEngineHost::engine(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_engines.size())) return nullptr;
    return m_engines[index].get();
}

QList<LuaFunctionDoc> ScriptEngineHost::functionDocs() const
{
    QList<LuaFunctionDoc> docs = LuaEngine::builtinFunctionDocs();
    for (auto& m : m_modules) docs.append(m->functions());
    return docs;
}

void ScriptEngineHost::setLoopValid(int index, bool valid)
{
    if (LuaEngine* e = engine(index)) e->SetLoopValid(valid);
}

void ScriptEngineHost::installModule(std::unique_ptr<ILuaBinding> module)
{
    if (!module) return;
    // 必须在脚本运行前注册:此刻不应有任何池任务在执行,否则 install 会与运行中的引擎竞争
    Q_ASSERT_X(!m_threadPool || m_threadPool->activeThreadCount() == 0,
               "ScriptEngineHost::installModule", "模块必须在脚本运行前注册");
    for (auto& e : m_engines)
        if (e) e->install(*module);
    m_modules.push_back(std::move(module));
}
