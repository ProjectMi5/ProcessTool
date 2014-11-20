#ifndef SKILLSTATEPOLLER_H
#define SKILLSTATEPOLLER_H

#include "uaclientsdk.h"
#include <QObject>
#include <QThread>
#include <QTimer>
#include <Mi5_ProcessTool/include/DataStructures.h>

class OpcuaGateway;
class ProductionModule;
class SkillStatePoller : public QObject
{
    Q_OBJECT
public:
    SkillStatePoller(ProductionModule* productionModule, int skillPos, OpcuaGateway* pGateway);
    ~SkillStatePoller();

public:

private slots:
    void checkSkillState();

private:
    void evalState();

private: //Qt
    QThread m_thread;
    QTimer* m_timer;

private:
    ProductionModule* m_pModule;
    OpcuaGateway* m_pGateway;
    int m_skillPos;
    OpcUa_Boolean m_busy;
    OpcUa_Boolean m_done;
    OpcUa_Boolean m_error;

};


#endif //SKILLSTATEPOLLER_H