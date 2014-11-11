#ifndef PRODUCTIONMODULE_H
#define PRODUCTIONMODULE_H
#include "uaclientsdk.h"
#include <iostream>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <Mi5_ProcessTool/include/ConnectionTestTimer.h>
#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>

class OpcuaGateway; // Using forward declaration.

class ProductionModule // Abstract base class.
    : public IProductionModule
{
protected:
    ModuleInput input;
    ModuleOutput output;

public:
    ProductionModule(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder);
    virtual ~ProductionModule();
    void subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
                                const UaDataNotifications& dataNotifications,
                                const UaDiagnosticInfos&   diagnosticInfos);
    void moduleDataChange(const UaDataNotifications& dataNotifications);

    void startup();
    //
    std::map<int, int> getSkills();
    int checkSkillState(int& SkillId);
    void writeModuleInput();
    void assignSkill(int& taskId, Skill skill, int& skillPos);
    void executeSkill(int& skillPos, ParameterInputArray& paramInput);
    void removeExecute(int& skillPos);
    UaString getSkillName(int& skillPos);
    UaString getModuleName();
    int getModulePosition();
    int registerTaskForSkill(ISkillRegistration* pTask, int skillPos);
    void deregisterTaskForSkill(int& skillPos);
    bool checkSkillReadyState(int& skillId);
    void writeConnectionTestInput(bool input);
    bool checkConnectionTestOutput();
    int translateSkillIdToSkillPos(int skillId);
    void serverReconnected();
    void moduleDisconnected();

private:
    OpcuaGateway* m_pOpcuaGateway;
    MessageFeeder* m_pMsgFeed;

private:
    UaString nodeIdToSubscribe;
    UaNodeIdArray nodeToSubscribe;
    int m_moduleNumber;
    ConnectionTestTimer* m_connectionTestTimer;

private:
    void createMonitoredItems();
    void createNodeStructure();
    void buildSkillList();
    void writeSkillInput(int skillPos);
    void skillStateChanged(int skillPos, int state);
    void setupOpcua();

private: //const
    static const int SKILLCOUNT = 16;

private: // module interface
    std::map<int, int> m_moduleSkillList;
    std::map<int, ISkillRegistration*> m_skillRegistrationList; /*skillPos, pTask*/

};

#endif // PRODUCTIONMODULE_H