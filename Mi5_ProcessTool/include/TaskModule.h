#ifndef TASKMODULE_H
#define TASKMODULE_H
#include "uaclientsdk.h"
#include <iostream>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IModule.h>
#include <Mi5_ProcessTool/include/Task.h>
#include <Mi5_ProcessTool/include/ProductionModule.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>

class OpcuaGateway; // Using forward declaration.

class TaskModule : public IModule
{

public:
    TaskModule(OpcuaGateway* pOpcuaGateway, int moduleNumber,
               std::map<int, IProductionModule*> moduleList, MessageFeeder* pMessageFeeder,
               IProductionModule* pManual);
    ~TaskModule();
    void subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
                                const UaDataNotifications& dataNotifications,
                                const UaDiagnosticInfos&   diagnosticInfos);
    void moduleDataChange(const UaDataNotifications& dataNotifications);
    void startup();
    void updateTaskStructure(ProductionTask& updatedTask, int skillNumberInTask);
    std::vector<skillModuleList> getSkillList();
    void notifyTaskDone(OpcUa_Int32& taskId, OpcUa_Int32& taskNumber);
    void serverReconnected();
    void updateTaskState(int taskNumber, TaskState state);

private:
    OpcuaGateway* m_pOpcuaGateway;

private:
    UaString nodeIdToSubscribe;
    UaNodeIdArray nodeToSubscribe;
    int m_moduleNumber;
    int m_taskCounter;
    std::map<int, Task*> m_taskObjects;
    std::map<OpcUa_Int32, ProductionTask> m_tasklist;
    std::map<int, IProductionModule*> m_moduleList;
    std::vector<skillModuleList> m_moduleSkillList;
    MessageFeeder* m_pMsgFeed;
    IProductionModule* m_pManual;

private:
    void createMonitoredItems();
    void createNodeStructure();
    UaStatus getTaskInformation(OpcUa_Int32 taskNumber);
    UaStatus writeTaskInformation(OpcUa_Int32 taskNumber, int skillNumberInTask);
    void buildSkillList();
    void setupOpcua();

private: //const
    static const int TASKCOUNT = 30;
    static const int SKILLCOUNT = 51;
    static const int PARAMETERCOUNT = 6;


};

#endif // TASKMODULE_H