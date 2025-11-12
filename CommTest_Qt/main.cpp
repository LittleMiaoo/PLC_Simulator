#include "CommTest_Qt.h"
#include <QtWidgets/QApplication>
#include <QStyleFactory>

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    

    //app.setStyle(QStyleFactory::create("Fusion"));
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    CommTest_Qt window;
   
    window.show();
    return app.exec();
}
