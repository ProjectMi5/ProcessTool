#ifndef RESETHELPER_H
#define RESETHELPER_H

#include <QObject>
#include <QApplication>
#include <QTimer>
#include <QThread>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <Mi5_ProcessTool/include/TaskModule.h>

//! tbd
/*!
    ..
*/
class ResetHelper : public QObject
{
    Q_OBJECT
public:
    ResetHelper(TaskModule* callbackPtr);
    ~ResetHelper();

public:

private:
    bool readExitDemand();
    bool resetExitDemand();

private slots:
    void timerTriggered();

private:
    TaskModule* m_pCallBack;
    QThread m_thread;
    QTimer* m_exitTimer;
    OpcuaGateway* m_pGateway;

signals:
    void resetTasks();
};

#endif // RESETHELPER_H