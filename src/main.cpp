/*
MIT License

Copyright (c) 2025-2026 Wang Mao

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

#include "MainWindow.h"
#include "I18n/LanguageManager.h"
#include "version.h"
#include <QtWidgets/QApplication>

#include <QCoreApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QSystemSemaphore>

#ifdef VLD_ENABLED
#define VLD_FORCE_ENABLE
#include <vld.h>
#endif

const QString SHARED_MEM_KEY = "PLC_Simulation_Shared_Memory";
const QString SEMAPHORE_KEY = "PLC_Simulation_Semaphore";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序元信息（用于 Qt 内部及系统集成）
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName(APP_ORGANIZATION);
    QCoreApplication::setOrganizationDomain(APP_DOMAIN);

    applyLanguage(app, resolveStartupLanguage());

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
            QMessageBox::critical(nullptr,
                                  QCoreApplication::translate("main", "错误"),
                                  QCoreApplication::translate("main", "创建共享内存失败！"));
            semaphore.release(); // 释放信号量
            return 1;
        }
    }

    semaphore.release(); // 释放信号量

    // 步骤3：已有实例，退出
    if (!isNewInstance) {
        QMessageBox::warning(nullptr,
                             QCoreApplication::translate("main", "提示"),
                             QCoreApplication::translate("main", "程序正在运行！请勿重复启动！"));
        return 0;
    }

    // 作用域块:让 MainWindow 先于 QApplication 析构
    int result = 0;
    {
        MainWindow window;
        window.show();

        result = app.exec();
    }

    return result;
}