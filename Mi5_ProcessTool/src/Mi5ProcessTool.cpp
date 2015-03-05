#include <Mi5_ProcessTool/include/ProcessHandler.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <Mi5_ProcessTool/include/QsLog/QsLogDest.h>
#include <Mi5_ProcessTool/include/HelperClasses/ExitHelper.h>
#include <QApplication>

static const int EXIT_CODE = 1337;
static const bool DEBUGTOCONSOLE = 1;
static const bool DEBUGTOVISUALSTUDIOOUTPUT = 0;

void logFunction(const QString& message, QsLogging::Level level)
{
    qDebug() << message;
}

void usage(char** argv)
{
    std::cout << "Usage for " << argv[0] << std::endl;
}

int main(int argc, char* argv[])
{
    bool initialInit = false;

    if (argc == 2)
    {
        initialInit = true;
        std::cout << "InitialInit set to true" << std::endl;
    }

    int currentExitCode = 0;

    QsLogging::Logger* pLogger = &QsLogging::Logger::instance();
    pLogger->setLoggingLevel(QsLogging::TraceLevel);

    if (DEBUGTOVISUALSTUDIOOUTPUT)
    {
        QsLogging::DestinationPtr debugDestination(
            QsLogging::DestinationFactory::MakeDebugOutputDestination());
        pLogger->addDestination(debugDestination);
    }

    if (DEBUGTOCONSOLE)
    {
        QsLogging::DestinationPtr functorDestination(QsLogging::DestinationFactory::MakeFunctorDestination(
                    &logFunction));
        pLogger->addDestination(functorDestination);
    }

    do
    {
        QApplication a(argc, argv);
        ProcessHandler* processHandler = new ProcessHandler(initialInit);
        ExitHelper* exitHelper = new ExitHelper();
        currentExitCode = a.exec();
    }
    while (currentExitCode == EXIT_CODE);

    return currentExitCode;
}