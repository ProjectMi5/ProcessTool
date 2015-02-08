#ifndef SKILLSTATEPOLLER_H
#define SKILLSTATEPOLLER_H

#include "uaclientsdk.h"
#include <QObject>
#include <QThread>
#include <QTimer>
#include <Mi5_ProcessTool/include/DataStructures.h>

class OpcuaGateway;
class IProductionModule;

//! The skill state poller provides a periodic check of a module's skill state upon construction.
/*!
    After a skill is being executed, an object of this class will be constructed to periodically check the
    skill's state. This has been implemented, because with the use of subscriptions, some state changes were missed.
*/
class SkillStatePoller : public QObject
{
    Q_OBJECT
public:
    SkillStatePoller(IProductionModule* productionModule, int skillPos, OpcuaGateway* pGateway);
    ~SkillStatePoller();

public:

private slots:
    virtual void checkSkillState();

protected:
    void evalState();

protected: //Qt
    QThread m_thread;
    QTimer* m_timer;

protected:
    IProductionModule* m_pModule;
    OpcuaGateway* m_pGateway;
    int m_skillPos;
    OpcUa_Boolean m_busy;
    OpcUa_Boolean m_done;
    OpcUa_Boolean m_error;
    OpcUa_Boolean m_ready;

};


#endif //SKILLSTATEPOLLER_H