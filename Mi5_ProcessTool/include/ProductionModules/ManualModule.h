#ifndef MANUALMODULE_H
#define MANUALMODULE_H

#include "uaclientsdk.h"

#include <Mi5_ProcessTool/include/IModule.h>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>

class OpcuaGateway;

class ManualModule : public IModule
{
public:
    ManualModule(OpcuaGateway* pOpcuaGateway, int moduleNumber,
                 MessageFeeder* pMessageFeeder);
    ~ManualModule();

public:
    virtual void subscriptionDataChange(OpcUa_UInt32 clientSubscriptionHandle,
                                        const UaDataNotifications& dataNotifications,
                                        const UaDiagnosticInfos&   diagnosticInfos);
    virtual void startup();
    virtual void serverReconnected();

private:
    void createMonitoredItems();
    void write();
    void moduleDataChange(const UaDataNotifications& dataNotifications);
    void createNodeStructure();
    void setupOpcua();

private:
    ManualModuleData data;
    OpcuaGateway* m_pGateway;
    int m_moduleNumber;
    UaNodeIdArray nodeToSubscribe;
    UaString nodeIdToSubscribe;

};



#endif // MANUALMODULE_H