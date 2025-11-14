#include "CommTest_Qt.h"
#include <QtWidgets/QApplication>
#include <QStyleFactory>

#ifdef _WIN32
#ifdef _DEBUG
#include "MemoryLeakDetector.h"
#endif
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
#ifdef _DEBUG
	// 启用内存泄漏检测
	MemoryLeakDetector::EnableMemoryLeakChecks();
	
	// 如果你知道特定的内存分配编号，可以设置断点
	 //MemoryLeakDetector::SetBreakAlloc(26543);  
#endif
#endif

    QApplication app(argc, argv);

    //app.setStyle(QStyleFactory::create("Fusion"));

    CommTest_Qt window;
   
    window.show();
    return app.exec();
}
