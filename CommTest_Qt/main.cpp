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

#ifdef _WIN32
#ifdef _DEBUG
#define VLD_FORCE_ENABLE
#include <vld.h>
#endif
#endif

const QString SHARED_MEM_KEY = "PLC_Simulation_Shared_Memory";
const QString SEMAPHORE_KEY = "PLC_Simulation_Semaphore";

int main(int argc, char *argv[])
{

#ifdef _WIN32
#ifdef _DEBUG
	// 启用内存泄漏检测
	MemoryLeakDetector::EnableMemoryLeakChecks();
	
	// 如果知道特定的内存分配编号，可以设置断点
	 //MemoryLeakDetector::SetBreakAlloc(15370);
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

    CommTest_Qt window;
    window.show();
    
    return app.exec();
}