#ifndef MAINTENANCEHELPER_H
#define MAINTENANCEHELPER_H
#include <iostream>


#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include "uaclientsdk.h"

#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/ProductionModule.h>
#include <Mi5_ProcessTool/include/GlobalConsts.h>
#include <Mi5_ProcessTool/include/IModule.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>

class MaintenanceHelper : public QObject, IModule, ISkillRegistration
{
    Q_OBJECT
public:
    MaintenanceHelper(MessageFeeder* pFeeder);
    ~MaintenanceHelper();

public: //IModule methods
    void subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
                                const UaDataNotifications& dataNotifications,
                                const UaDiagnosticInfos&   diagnosticInfos);
    void startup();
    void serverReconnected();

public: //ISkillRegistration methods
    int getTaskId();
    void skillStateChanged(int moduleNumber, int skillPos, int state);

public:
    void maintain(int moduleNumber, int errorId);
    void setModuleList(std::map<int, IProductionModule*> pModuleList);

private:
    void resetData();
    void maintenanceExecution(int moduleNumber, int errorId);
    void fillParams(ParameterInputArray& tmpParamArray, int errorId);

private:
    std::map<int, IProductionModule*> m_pModuleList;
    QThread m_thread;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    MessageFeeder* m_pMsgFeeder;

private:
    int m_moduleToMaintain;
    bool m_maintenanceInProcess;

};

#endif //MAINTENANCEHELPER_H