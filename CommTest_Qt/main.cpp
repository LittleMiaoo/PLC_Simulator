/*
MIT License

Copyright (c) 2025-2026 Wang Mao <mao.wang.dev@foxmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "CommTest_Qt.h"
#include "version.h"
#include <QtWidgets/QApplication>
#include <QStyleFactory>

#include <QMessageBox>
#include <QSharedMemory>
#include <QSystemSemaphore>

#ifdef _WIN32
#ifdef _DEBUG
#include "MemoryLeakDetector.h"
#endif
#endif

#ifdef VLD_ENABLED
#define VLD_FORCE_ENABLE
#include <vld.h>
#endif

const QString SHARED_MEM_KEY = "PLC_Simulation_Shared_Memory";
const QString SEMAPHORE_KEY = "PLC_Simulation_Semaphore";

int main(int argc, char *argv[])
{

#ifndef VLD_ENABLED
	// 如果没有启用VLD，则使用CRT内存检测
	#ifdef _WIN32
	#ifdef _DEBUG
		// 启用内存泄漏检测
		MemoryLeakDetector::EnableMemoryLeakChecks();

		// 创建初始内存快照(在创建任何对象之前)
		_CrtMemState memStateStart;
		_CrtMemCheckpoint(&memStateStart);

		// 设置断点在第一个泄漏的内存分配上
		// 从最新的泄漏报告中选择最早的泄漏块编号
		//MemoryLeakDetector::SetBreakAlloc(178);
	#endif
	#endif
#endif

    //int* testLeak = new int(42);  // 故意制造内存泄漏以测试检测功能
    QApplication app(argc, argv);

    // 设置应用程序元信息（用于 Qt 内部及系统集成）
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName(APP_ORGANIZATION);
    QCoreApplication::setOrganizationDomain(APP_DOMAIN);

    //app.setStyle(QStyleFactory::create("Fusion"));

    // 步骤1：创建系统信号量（防止多实例同时检查共享内存）
    QSystemSemaphore semaphore(SEMAPHORE_KEY, 1);
    semaphore.acquire(); // 加锁，独占检查

    // 步骤2：检查共享内存是否存在
    QSharedMemory sharedMem(SHARED_MEM_KEY);
    bool isNewInstance = false;
    if (!sharedMem.attach()) {
        // 无现有实例，创建共享内存（大小任意，仅作标识）
        if (sharedMem.create(1)) {
            isNewInstance = true;
        } else {
            QMessageBox::critical(nullptr, "错误", "创建共享内存失败！");
            semaphore.release(); // 释放信号量
            return 1;
        }
    }

    semaphore.release(); // 释放信号量

    // 步骤3：已有实例，退出
    if (!isNewInstance) {
        QMessageBox::warning(nullptr, "提示", "程序正在运行！请勿重复启动！");
        return 0;
    }

    int result = 0;
    {
        CommTest_Qt window;
        window.show();

        result = app.exec();

        // window析构发生在这里
    }

    // 确保QApplication完全清理
    // app析构将在return之前发生

#ifndef VLD_ENABLED
	// 如果没有启用VLD，则使用CRT内存检测
	#ifdef _WIN32
	#ifdef _DEBUG
		// 创建结束内存快照
		_CrtMemState memStateEnd, memStateDiff;
		_CrtMemCheckpoint(&memStateEnd);

		// 比较开始和结束的内存状态,只报告差异
		if (_CrtMemDifference(&memStateDiff, &memStateStart, &memStateEnd))
		{
			_CrtMemDumpStatistics(&memStateDiff);
		}
	#endif
	#endif
#endif

    return result;
}