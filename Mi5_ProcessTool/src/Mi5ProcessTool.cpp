#include <Mi5_ProcessTool/include/ProcessHandler.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <Mi5_ProcessTool/include/QsLog/QsLogDest.h>
#include <Mi5_ProcessTool/include/HelperClasses/ExitHelper.h>
#include <QApplication>

static const int EXIT_CODE = 1337;

int main(int argc, char* argv[])
{
    int currentExitCode = 0;


    do
    {
        QApplication a(argc, argv);
        ExitHelper* exitHelper = new ExitHelper();
        QsLogging::Logger* pLogger = &QsLogging::Logger::instance();
        pLogger->setLoggingLevel(QsLogging::TraceLevel);
        QsLogging::DestinationPtr debugDestination(
            QsLogging::DestinationFactory::MakeDebugOutputDestination());
        pLogger->addDestination(debugDestination);

        ProcessHandler* processHandler = new ProcessHandler();
        currentExitCode = a.exec();
    }
    while (currentExitCode == EXIT_CODE);

    return currentExitCode;
}