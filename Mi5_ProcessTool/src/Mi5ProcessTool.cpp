#include <Mi5_ProcessTool/include/ProcessHandler.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <Mi5_ProcessTool/include/QsLog/QsLogDest.h>
#include <QApplication>
#include <qthreadpool.h>

static const int EXIT_CODE = 1337;
static const bool DEBUGTOCONSOLE = 1;
static const bool DEBUGTOVISUALSTUDIOOUTPUT = 1;

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

    int currentExitCode = 0;
    QThreadPool::globalInstance()->setMaxThreadCount(128);
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
        ProcessHandler* processHandler = new ProcessHandler();
        currentExitCode = a.exec();
    }
    while (currentExitCode == EXIT_CODE);

    return currentExitCode;
}