#include <Mi5_ProcessTool/include/OpcuaSubscriber.h>
#include <uasubscription.h>
#include <uasession.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

OpcuaSubscriber::OpcuaSubscriber()
{
    m_subscriptionList.clear();
}

OpcuaSubscriber::~OpcuaSubscriber()
{
    //if (m_pSubscription)
    //{
    //    deleteSubscription();
    //}
    //TODO: Delete subscriptions
}

void OpcuaSubscriber::subscriptionStatusChanged(
    OpcUa_UInt32
    clientSubscriptionHandle, //!< [in] Client defined handle of the affected subscription
    const UaStatus&   status)                   //!< [in] Changed status for the subscription
{
    OpcUa_ReferenceParameter(
        clientSubscriptionHandle); // We use the callback only for this subscription

    QLOG_ERROR() << "Subscription not longer valid - failed with status " << status.toString().toUtf8();
}

void OpcuaSubscriber::dataChange(
    OpcUa_UInt32
    clientSubscriptionHandle, //!< [in] Client defined handle of the affected subscription
    const UaDataNotifications&
    dataNotifications,        //!< [in] List of data notifications sent by the server
    const UaDiagnosticInfos&
    diagnosticInfos)          //!< [in] List of diagnostic info related to the data notifications. This list can be empty.
{
    m_pOpcuaGateway->subscriptionDataChange(clientSubscriptionHandle, dataNotifications,
                                            diagnosticInfos);

}

void OpcuaSubscriber::newEvents(
    OpcUa_UInt32
    clientSubscriptionHandle, //!< [in] Client defined handle of the affected subscription
    UaEventFieldLists&
    eventFieldList)           //!< [in] List of event notifications sent by the server
{
    OpcUa_ReferenceParameter(clientSubscriptionHandle);
    OpcUa_ReferenceParameter(eventFieldList);
}

UaStatus OpcuaSubscriber::createSubscription(UaSession* pSession,
        OpcUa_UInt32 subscriptionClientHandle)
{
    if (m_subscriptionList.count(subscriptionClientHandle) > 0)
    {
        printf("\nError: Subscription already created\n");
        return OpcUa_BadInvalidState;
    }

    UaStatus result;

    ServiceSettings serviceSettings;
    SubscriptionSettings subscriptionSettings;
    subscriptionSettings.publishingInterval = 1;
    int deleteme = m_subscriptionList.size();

    m_subscriptionList[subscriptionClientHandle] = NULL;
    UaSubscription* tempSubscription;
    QLOG_DEBUG() << "Creating subscription with clienthandle " << subscriptionClientHandle << "..";
    result = pSession->createSubscription(
                 serviceSettings,
                 this,
                 subscriptionClientHandle,
                 subscriptionSettings,
                 OpcUa_True,
                 &tempSubscription);
    m_subscriptionList[subscriptionClientHandle] = tempSubscription;

    if (result.isGood())
    {
        QLOG_DEBUG() << "CreateSubscription for clienthandle " << subscriptionClientHandle << " succeeded.";
    }
    else
    {
        QLOG_ERROR() << "CreateSubscription failed with status " << result.toString().toUtf8();
    }

    return result;
}

UaStatus OpcuaSubscriber::deleteSubscription(UaSession* pSession,
        OpcUa_UInt32 subscriptionClientHandle)
{
    if (m_subscriptionList.count(subscriptionClientHandle) != 1)
    {
        printf("\nError: No Subscription created\n");
        return OpcUa_BadInvalidState;
    }

    UaStatus result;
    ServiceSettings serviceSettings;

    // let the SDK cleanup the resources for the existing subscription
    printf("\nDeleting subscription ...\n");
    result = pSession->deleteSubscription(
                 serviceSettings,
                 &m_subscriptionList[subscriptionClientHandle]);

    if (result.isGood())
    {
        printf("DeleteSubscription succeeded\n");
    }
    else
    {
        QLOG_ERROR() << "DeleteSubscription failed with status " << result.toString().toUtf8();
    }

    return result;
}

UaStatus OpcuaSubscriber::deleteAllSubscriptions(UaSession* pSession)
{
    if (m_subscriptionList.size() == 0)
    {
        printf("\nError: No Subscription created\n");
        return OpcUa_BadInvalidState;
    }

    UaStatus result;
    ServiceSettings serviceSettings;

    for (std::map<int, UaSubscription*>::iterator it = m_subscriptionList.begin();
         it != m_subscriptionList.end(); it++)
    {

        // let the SDK cleanup the resources for the existing subscription
        result = pSession->deleteSubscription(
                     serviceSettings,
                     &m_subscriptionList[it->first]);

        if (!result.isGood())
        {
            QLOG_ERROR() << "DeleteSubscription failed with status " << result.toString().toUtf8();
        }
    }

    m_subscriptionList.clear();
    return result;
}

UaStatus OpcuaSubscriber::createMonitoredItems(UaNodeIdArray nodesToSubscribe)
{
    //if (m_subscriptionList.count(subscriptionClientHandle) > 0)
    //{
    //    printf("\nError: No Subscription created\n");
    //    return OpcUa_BadInvalidState;
    //}

    //UaStatus result;
    //OpcUa_UInt32 i;
    //ServiceSettings serviceSettings;
    //UaMonitoredItemCreateRequests itemsToCreate;
    //UaMonitoredItemCreateResults createResults;

    //// Configure one item to add to subscription
    ///*// We monitor the value of the ServerStatus -> CurrentTime
    //itemsToCreate.create(1);
    //itemsToCreate[0].ItemToMonitor.AttributeId = OpcUa_Attributes_Value;
    //itemsToCreate[0].ItemToMonitor.NodeId.Identifier.Numeric = OpcUaId_Server_ServerStatus_CurrentTime;
    //itemsToCreate[0].RequestedParameters.ClientHandle = 1;
    //itemsToCreate[0].RequestedParameters.SamplingInterval = 100;
    //itemsToCreate[0].RequestedParameters.QueueSize = 1;
    //itemsToCreate[0].RequestedParameters.DiscardOldest = OpcUa_True;
    //itemsToCreate[0].MonitoringMode = OpcUa_MonitoringMode_Reporting;*/


    //itemsToCreate.create(nodesToSubscribe.length());

    //for (OpcUa_UInt32 i = 0; i < nodesToSubscribe.length(); i++)
    //{
    //    itemsToCreate[i].ItemToMonitor.AttributeId = OpcUa_Attributes_Value;
    //    OpcUa_NodeId_CopyTo(&nodesToSubscribe[i], &itemsToCreate[i].ItemToMonitor.NodeId);
    //    itemsToCreate[i].RequestedParameters.ClientHandle = 1336;
    //    itemsToCreate[i].RequestedParameters.SamplingInterval = 1;
    //    itemsToCreate[i].RequestedParameters.QueueSize = 1;
    //    itemsToCreate[i].RequestedParameters.DiscardOldest = OpcUa_True;
    //    itemsToCreate[i].MonitoringMode = OpcUa_MonitoringMode_Reporting;
    //}


    //printf("\nAdding monitored items to subscription ...\n");
    //result = m_subscriptionList[modulenum->createMonitoredItems(
    //             serviceSettings,
    //             OpcUa_TimestampsToReturn_Both,
    //             itemsToCreate,
    //             createResults);

    //if (result.isGood())
    //{
    //    // check individual results
    //    for (i = 0; i < createResults.length(); i++)
    //    {
    //        if (OpcUa_IsGood(createResults[i].StatusCode))
    //        {
    //            printf("CreateMonitoredItems succeeded for item: %s\n",
    //                   UaNodeId(itemsToCreate[i].ItemToMonitor.NodeId).toXmlString().toUtf8());
    //        }
    //        else
    //        {
    //            printf("CreateMonitoredItems failed for item: %s - Status %s\n",
    //                   UaNodeId(itemsToCreate[i].ItemToMonitor.NodeId).toXmlString().toUtf8(),
    //                   UaStatus(createResults[i].StatusCode).toString().toUtf8());
    //        }
    //    }
    //}
    //// service call failed
    //else
    //{
    //    printf("CreateMonitoredItems failed with status %s\n", result.toString().toUtf8());
    //}

    //return result;
    UaStatus result;
    return result;
}

UaStatus OpcuaSubscriber::createSingleMonitoredItem(OpcUa_UInt32 clientHandle,
        OpcUa_UInt32 subscriptionClientHandle,
        UaNodeIdArray nodesToSubscribe)
{
    if (m_subscriptionList.count(subscriptionClientHandle) != 1)
    {
        QLOG_ERROR() << "\nError: No Subscription created\n";
        return OpcUa_BadInvalidState;
    }

    if (nodesToSubscribe.length() != 1)
    {
        QLOG_ERROR() << "Error, Length of nodesToSubscribe not equal to 1." ;
        return OpcUa_BadInvalidState;
    }

    UaStatus result;
    ServiceSettings serviceSettings;
    UaMonitoredItemCreateRequests itemsToCreate;
    UaMonitoredItemCreateResults createResults;

    itemsToCreate.create(1);
    itemsToCreate[0].ItemToMonitor.AttributeId = OpcUa_Attributes_Value;
    OpcUa_NodeId_CopyTo(&nodesToSubscribe[0], &itemsToCreate[0].ItemToMonitor.NodeId);
    itemsToCreate[0].RequestedParameters.ClientHandle = clientHandle;
    itemsToCreate[0].RequestedParameters.SamplingInterval = 1;
    itemsToCreate[0].RequestedParameters.QueueSize = 1;
    itemsToCreate[0].RequestedParameters.DiscardOldest = OpcUa_True;
    itemsToCreate[0].MonitoringMode = OpcUa_MonitoringMode_Reporting;

    result = m_subscriptionList[subscriptionClientHandle]->createMonitoredItems(
                 serviceSettings,
                 OpcUa_TimestampsToReturn_Both,
                 itemsToCreate,
                 createResults);

    if (!result.isGood())
    {
        printf("CreateMonitoredItems failed for item: %s - Status %s\n",
               UaNodeId(itemsToCreate[0].ItemToMonitor.NodeId).toXmlString().toUtf8(),
               UaStatus(createResults[0].StatusCode).toString().toUtf8());
        QLOG_ERROR() << "CreateMonitoredItems failed with status " << result.toString().toUtf8();
    }

    // QLOG_DEBUG() << "Created subscription with clienthandle " << clientHandle;
    return result;
}


void OpcuaSubscriber::setGateway(OpcuaGateway* gateway)
{
    m_pOpcuaGateway = gateway;
}
