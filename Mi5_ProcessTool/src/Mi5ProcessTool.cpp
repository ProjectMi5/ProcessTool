#include <Mi5_ProcessTool/include/ProcessHandler.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <Mi5_ProcessTool/include/QsLog/QsLogDest.h>
#include <QApplication>



int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    //Gui gui;
    //gui.show();

    QsLogging::Logger* pLogger = &QsLogging::Logger::instance();
    pLogger->setLoggingLevel(QsLogging::TraceLevel);
    QsLogging::DestinationPtr debugDestination(
        QsLogging::DestinationFactory::MakeDebugOutputDestination());
    pLogger->addDestination(debugDestination);

    ProcessHandler* processHandler = new ProcessHandler();

    //delete processHandler;
    //processHandler = NULL;
    int applicationResult = a.exec();
    return applicationResult;

}