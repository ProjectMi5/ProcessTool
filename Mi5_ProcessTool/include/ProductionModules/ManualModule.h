#ifndef MANUALMODULE_H
#define MANUALMODULE_H

#include "uaclientsdk.h"

#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>

class OpcuaGateway;

class ManualModule : public IProductionModule
{
public:
    ManualModule(OpcuaGateway* pOpcuaGateway, int moduleNumber,
                 MessageFeeder* pMessageFeeder);
    ~ManualModule();

public: //IModule methods
    virtual void subscriptionDataChange(OpcUa_UInt32 clientSubscriptionHandle,
                                        const UaDataNotifications& dataNotifications,
                                        const UaDiagnosticInfos&   diagnosticInfos);
    virtual void startup();
    virtual void serverReconnected();

public: //IProductionModule Methods
    std::map<int, int> getSkills();
    int checkSkillState(int& skillId);
    bool checkSkillReadyState(int& skillId);
    int translateSkillIdToSkillPos(int skillId);
    virtual int translateSkillPosToSkillId(int skillPos);
    void assignSkill(int& taskId, Skill skill, int& skillPos);
    void executeSkill(int& skillPos, ParameterInputArray& paramInput);
    void deregisterTaskForSkill(int& skillPos);
    UaString getSkillName(int& skillPos);
    UaString getModuleName();
    int getModulePosition();
    int registerTaskForSkill(ISkillRegistration* pTask, int skillPos);
    void writeConnectionTestInput(bool input);
    int checkConnectionTestOutput();
    void moduleConnectionStatusChanged(int state);
    virtual bool isBlocked();
    virtual bool isReserved();
    virtual void changeModuleMode(int mode);

private:
    void createMonitoredItems();
    void write();
    void moduleDataChange(const UaDataNotifications& dataNotifications);
    void createNodeStructure();
    void setupOpcua();
    void skillStateChanged(int skillPos, int state);

private:
    ManualModuleData data;
    OpcuaGateway* m_pGateway;
    int m_moduleNumber;
    UaNodeIdArray nodeToSubscribe;
    UaString nodeIdToSubscribe;
    std::map<int, ISkillRegistration*> m_skillRegistrationList; /*skillPos, pTask*/

};



#endif // MANUALMODULE_H