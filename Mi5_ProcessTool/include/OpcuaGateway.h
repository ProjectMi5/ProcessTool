#ifndef OPCUAGATEWAY_H
#define OPCUAGATEWAY_H

#include "uasession.h"
#include "uabase.h"
#include "uaclientsdk.h"
#include <iostream>

#include <Mi5_ProcessTool/include/UaHelper.h>
#include <Mi5_ProcessTool/include/OpcuaConfigurator.h>
#include <Mi5_ProcessTool/include/OpcuaSubscriber.h>
#include <Mi5_ProcessTool/include/IModule.h>

using namespace UaClientSdk;

class OpcuaGateway : public UaSessionCallback
{
    UA_DISABLE_COPY(OpcuaGateway);
public:
    OpcuaGateway(UaString serverUrl);
    virtual ~OpcuaGateway();
    UaStatus loadConfig();

    // UaSessionCallback implementation ----------------------------------------------------
    virtual void connectionStatusChanged(OpcUa_UInt32 clientConnectionId,
                                         UaClient::ServerStatus serverStatus);

    // OPC UA service calls
    UaStatus connect();
    UaStatus disconnect();
    UaStatus browseSimple();
    UaStatus browseContinuationPoint();
    UaDataValues read(UaReadValueIds& nodesToRead, OpcUa_Int32 timeout = 10000);
    UaStatus write(UaWriteValues& nodesToWrite, OpcUa_Int32 timeout = 10000);
    UaStatus subscribe(int subscriptionHandle, UaNodeIdArray nodesToSubscribe);
    UaStatus unsubscribe();

    UaStatus createSubscription(int subscriptionHandle);
    UaStatus deleteSubscription(int subscriptionHandle);
    UaStatus createSingleMonitoredItem(OpcUa_UInt32 clientHandle, OpcUa_UInt32 subscriptionHandle,
                                       UaNodeIdArray nodesToSubscribe);

    void subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
                                const UaDataNotifications& dataNotifications,
                                const UaDiagnosticInfos&   diagnosticInfos);
    void registerModule(int moduleNumber, IModule* pModule);
    UaString getServerUrl();

private:
    // helper methods
    UaStatus browseInternal(const UaNodeId& nodeToBrowse, OpcUa_UInt32 maxReferencesToReturn);
    void printBrowseResults(const UaReferenceDescriptions& referenceDescriptions);

private:
    UaSession*                      m_pSession;
    OpcuaSubscriber*                m_pSubscription;
    OpcuaConfigurator*              m_pConfiguration;
    UaClient::ServerStatus          m_serverStatus;
    OpcuaGateway*                   m_pOpcuaGateway;
    std::map<int, IModule*> m_moduleList;
    UaString                        m_serverUrl;
};


#endif // OPCUAGATEWAY_H

