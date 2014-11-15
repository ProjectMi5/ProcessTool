#ifndef OPCUASUBSCRIBER_H
#define OPCUASUBSCRIBER_H

#include "uabase.h"
#include "uaclientsdk.h"

using namespace UaClientSdk;

class OpcuaGateway;
class OpcuaSubscriber :
    public UaSubscriptionCallback
{
    UA_DISABLE_COPY(OpcuaSubscriber);
public:
    OpcuaSubscriber();
    virtual ~OpcuaSubscriber();

    // UaSubscriptionCallback implementation ----------------------------------------------------
    virtual void subscriptionStatusChanged(
        OpcUa_UInt32      clientSubscriptionHandle,
        const UaStatus&   status);
    virtual void dataChange(
        OpcUa_UInt32               clientSubscriptionHandle,
        const UaDataNotifications& dataNotifications,
        const UaDiagnosticInfos&   diagnosticInfos);
    virtual void newEvents(
        OpcUa_UInt32                clientSubscriptionHandle,
        UaEventFieldLists&          eventFieldList);
    virtual void notificationsMissing(OpcUa_UInt32 clientSubscriptionHandle,
                                      OpcUa_UInt32 previousSequenceNumber, OpcUa_UInt32 newSequenceNumber);
    // UaSubscriptionCallback implementation ------------------------------------------------------

    // Create / delete a subscription on the server
    UaStatus createSubscription(UaSession* pSession, OpcUa_UInt32 subscriptionClientHandle);
    UaStatus deleteSubscription(UaSession* pSession,
                                OpcUa_UInt32 subscriptionClientHandle);
    UaStatus deleteAllSubscriptions(UaSession* pSession);

    // Create monitored items in the subscription
    UaStatus createMonitoredItems(UaNodeIdArray nodesToSubscribe);
    UaStatus createSingleMonitoredItem(OpcUa_UInt32 clientHandle,
                                       OpcUa_UInt32 subscriptionClientHandle,
                                       UaNodeIdArray nodesToSubscribe);

    void setGateway(OpcuaGateway* gateway);

private:
    OpcuaGateway*               m_pOpcuaGateway;
    std::map<int, UaSubscription*> m_subscriptionList;
};

#endif // OPCUASUBSCRIBER_H
