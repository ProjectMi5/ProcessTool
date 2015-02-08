#ifndef EXITHELPER_H
#define EXITHELPER_H

#include <QObject>
#include <QApplication>
#include <QTimer>
#include <QThread>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>

//! tbd
/*!
    ..
*/
class ExitHelper : public QObject
{
    Q_OBJECT
public:
    ExitHelper();
    ~ExitHelper();

public:

private:
    bool readExitDemand();
    bool resetExitDemand();
    void quitApplication();

private slots:
    void timerTriggered();

private:
    QThread m_thread;
    QTimer* m_exitTimer;
    OpcuaGateway* m_pGateway;
};

#endif // EXITHELPER_H