#ifndef PRODUCTIONMODULE_H
#define PRODUCTIONMODULE_H
#include "uaclientsdk.h"
#include <iostream>
#include <QObject>
#include <QThread>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <Mi5_ProcessTool/include/ConnectionTestTimer.h>
#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>
#include <Mi5_ProcessTool/include/SkillStatePoller.h>
#include <Mi5_ProcessTool/include/ProductionModule.h>
#include <Mi5_ProcessTool/include/PositionCalibrator.h>

class OpcuaGateway; // Using forward declaration.
class MaintenanceHelper;
class InitManager;

class ProductionModule  : public QObject, public IProductionModule
{
    Q_OBJECT
protected:
    ModuleInput input;
    ModuleOutput output;
    int m_moduleNumber;

public:
    ProductionModule(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
                     MaintenanceHelper* pMaintenanceHelper, InitManager* pInitManager);
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
    double getModulePosition();
    int registerTaskForSkill(ISkillRegistration* pTask, int skillPos);
    void deregisterTaskForSkill(int& skillPos);
    bool checkSkillReadyState(int& skillId);
    void writeConnectionTestInput(bool input);
    int checkConnectionTestOutput();
    int translateSkillIdToSkillPos(int skillId);
    virtual bool isBlocked();
    virtual bool isReserved();
    int translateSkillPosToSkillId(int skillPos);
    virtual void changeModuleMode(int mode);
    UaString getBaseNodeId();

public slots:
    void moduleConnectionStatusChanged(int state);
    void serverReconnected();
    void skillStateChanged(int skillPos, int state);

protected:
    OpcuaGateway* m_pOpcuaGateway;
    MessageFeeder* m_pMsgFeed;
    bool m_disconnected;
    UaString m_baseNodeId;
    MaintenanceHelper* m_pMaintenanceHelper;
    InitManager* m_pInitManager;

private:
    UaString nodeIdToSubscribe;
    UaNodeIdArray nodeToSubscribe;
    ConnectionTestTimer* m_connectionTestTimer;
    QThread m_thread;

private slots:
    void createPoller(int skillPos);

private:
    void createMonitoredItems();
    void createNodeStructure();
    void buildSkillList();
    void writeSkillInput(int skillPos);
    void setupOpcua();
    virtual void checkMoverState(int skillPos);

protected slots:
    virtual void evaluateError() = 0;

private: //const
    static const int SKILLCOUNT = 16;

private: // module interface
    std::map<int, int> m_moduleSkillList; /* skillid, skillpos */
    std::map<int, ISkillRegistration*> m_skillRegistrationList; /*skillPos, pTask*/
    std::map<int, SkillStatePoller*> m_skillStatePollerList; /* skillPos, pSkillStaterPoller */

signals:
    void errorOccured();
    void skillStateChangeSignal(int moduleNumber, int skillPos, int state);

};

#endif // PRODUCTIONMODULE_H