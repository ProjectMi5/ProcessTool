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

//! ProductionModule provides the main functionalities for the production modules.
/*!
    Detailed descriptions goes here.
*/
class ProductionModule  : public QObject, public IProductionModule
{
    Q_OBJECT

protected:
    ModuleInput input;
    ModuleOutput output;
    int m_moduleNumber;
    OpcuaGateway* m_pOpcuaGateway;
    MessageFeeder* m_pMsgFeed;
    bool m_disconnected;
    UaString m_baseNodeId;
    MaintenanceHelper* m_pMaintenanceHelper;
    InitManager* m_pInitManager;
    bool m_enableConnectionTest;

public:
    // Ctor/Dtor
    ProductionModule(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
                     MaintenanceHelper* pMaintenanceHelper, InitManager* pInitManager);
    virtual ~ProductionModule();

    // Implementation of the IModule interface.
    void subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
                                const UaDataNotifications& dataNotifications,
                                const UaDiagnosticInfos&   diagnosticInfos);
    void moduleDataChange(const UaDataNotifications& dataNotifications);
    void startup();

    // Implementation of the IProductionModule interface.
    std::map<int, int> getSkills();
    int checkSkillState(int& SkillId);
    bool checkSkillReadyState(int& skillId);
    int translateSkillIdToSkillPos(int skillId);
    int translateSkillPosToSkillId(int skillPos);
    void assignSkill(int& taskId, Skill skill, int& skillPos);
    void executeSkill(int& skillPos, ParameterInputArray& paramInput);
    void deregisterTaskForSkill(int& skillPos);
    UaString getSkillName(int& skillPos);
    UaString getModuleName();
    double getModulePosition();
    int registerTaskForSkill(ISkillRegistration* pTask, int skillPos);
    void writeConnectionTestInput(bool input);
    int checkConnectionTestOutput();
    //
    virtual bool isBlocked();
    virtual bool isReserved();
    virtual void changeModuleMode(int mode);
    UaString getBaseNodeId();
    //

    // Other methods.
    void writeModuleInput();
    void removeExecute(int& skillPos);
    int getErrorId();

public slots:
    void moduleConnectionStatusChanged(int state);
    void serverReconnected();
    void skillStateChanged(int skillPos, int state);

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
    std::map<int, int> m_moduleSkillList; /*!< skillid, skillpos */
    std::map<int, ISkillRegistration*> m_skillRegistrationList; /*!< skillPos, pTask*/
    std::map<int, SkillStatePoller*> m_skillStatePollerList; /*!< skillPos, pSkillStaterPoller */

signals:
    void errorOccured();
    void skillStateChangeSignal(int moduleNumber, int skillPos, int state);

};

#endif // PRODUCTIONMODULE_H