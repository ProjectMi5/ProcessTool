#ifndef MANUALMODULE_H
#define MANUALMODULE_H

#include "uaclientsdk.h"

#include <QObject>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>
#include <Mi5_ProcessTool/include/ProductionModules/SkillStatePollerManual.h>

class OpcuaGateway;

class ManualModule : public QObject, public IProductionModule
{
    Q_OBJECT
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
    double getModulePosition();
    int registerTaskForSkill(ISkillRegistration* pTask, int skillPos);
    void writeConnectionTestInput(bool input);
    int checkConnectionTestOutput();
    void moduleConnectionStatusChanged(int state);
    virtual bool isBlocked();
    virtual bool isReserved();
    virtual void changeModuleMode(int mode);
    UaString getBaseNodeId();

private:
    void createMonitoredItems();
    void write();
    void moduleDataChange(const UaDataNotifications& dataNotifications);
    void createNodeStructure();
    void setupOpcua();

private slots:
    void createPoller(int skillPos);

public slots:
    void skillStateChanged(int skillPos, int state);

private:
    ManualModuleData data;
    OpcuaGateway* m_pGateway;
    int m_moduleNumber;
    UaNodeIdArray nodeToSubscribe;
    UaString nodeIdToSubscribe;
    std::map<int, ISkillRegistration*> m_skillRegistrationList; /*skillPos, pTask*/
    std::map<int, SkillStatePoller*> m_skillStatePollerList; /* skillPos, pSkillStaterPoller */

};



#endif // MANUALMODULE_H