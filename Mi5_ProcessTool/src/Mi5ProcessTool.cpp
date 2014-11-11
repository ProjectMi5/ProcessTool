#include <Mi5_ProcessTool/include/ProcessHandler.h>
#include <Mi5_ProcessTool/include/Gui.h>

#include <QApplication>



int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    //Gui gui;
    //gui.show();

    ProcessHandler* processHandler = new ProcessHandler();



    //delete processHandler;
    //processHandler = NULL;
    int applicationResult = a.exec();
    return applicationResult;

}