#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <Mi5_ProcessTool/include/ProductionModule.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

OpcuaGateway::OpcuaGateway(UaString serverUrl)
{
    m_moduleList.clear();

    m_pSession = new UaSession();
    m_pConfiguration = new OpcuaConfigurator();

    m_pSubscription = new OpcuaSubscriber();
    m_pSubscription->setGateway(this);

    m_serverUrl = serverUrl;
}

OpcuaGateway::~OpcuaGateway()
{
    if (m_pSession)
    {
        // delete local subscription object
        delete m_pSession;
        m_pSession = NULL;
    }

    if (m_pConfiguration)
    {
        delete m_pConfiguration;
        m_pConfiguration = NULL;
    }
}

UaString OpcuaGateway::getServerUrl()
{
    return m_serverUrl;
}

UaStatus OpcuaGateway::loadConfig()
{
    UaString sConfigFile(getAppPath());
    sConfigFile += "\\OPCUAconfig.ini"; // Windows style
    QLOG_DEBUG() << "Loading configuration file.." ;
    UaStatus status = m_pConfiguration->loadConfiguration(sConfigFile);

    if (status.isGood())
    {
        QLOG_DEBUG() << "Configuration file loaded successfully." ;

    }
    else
    {
        QLOG_DEBUG() << "Error loading configuration file." ;
    }

    return status;
}

void OpcuaGateway::connectionStatusChanged(
    OpcUa_UInt32             clientConnectionId,
    UaClient::ServerStatus   serverStatus)
{
    OpcUa_ReferenceParameter(clientConnectionId);

    switch (serverStatus)
    {
    case UaClient::Disconnected:
        QLOG_INFO() << "Connection status changed to Disconnected\n";

        // Delete subscriptions for the connected module.
        for (std::map<int, IModule*>::iterator it = m_moduleList.begin(); it != m_moduleList.end(); it++)
        {
            m_pSubscription->deleteAllSubscriptions(m_pSession);
        }

        break;

    case UaClient::Connected:
        QLOG_INFO() << "Connection status changed to Connected\n";

        if (m_serverStatus != UaClient::NewSessionCreated)
        {
            m_pConfiguration->updateNamespaceIndexes(m_pSession->getNamespaceTable());

            // Renew subscriptions for the connected module.
            for (std::map<int, IModule*>::iterator it = m_moduleList.begin(); it != m_moduleList.end(); it++)
            {
                m_pSubscription->deleteAllSubscriptions(m_pSession);
                it->second->serverReconnected();
            }
        }


        break;

    case UaClient::ConnectionWarningWatchdogTimeout:
        QLOG_INFO() << "Connection status changed to ConnectionWarningWatchdogTimeout\n";
        break;

    case UaClient::ConnectionErrorApiReconnect:
        QLOG_INFO() << "Connection status changed to ConnectionErrorApiReconnect\n";
        break;

    case UaClient::ServerShutdown:
        QLOG_INFO() << "Connection status changed to ServerShutdown\n";
        break;

    case UaClient::NewSessionCreated:
        QLOG_INFO() << "Connection status changed to NewSessionCreated\n";
        m_pConfiguration->updateNamespaceIndexes(m_pSession->getNamespaceTable());
        break;
    }

    m_serverStatus = serverStatus;
}


UaStatus OpcuaGateway::connect()
{
    UaStatus result;

    // Provide information about the client
    SessionConnectInfo sessionConnectInfo;
    UaString sNodeName("unknown_host");
    char szHostName[256];

    if (0 == UA_GetHostname(szHostName, 256))
    {
        sNodeName = szHostName;
    }

    // Fill session connect info with data from configuration
    sessionConnectInfo.sApplicationName = m_pConfiguration->getApplicationName();
    // Use the host name to generate a unique application URI
    sessionConnectInfo.sApplicationUri = UaString("urn:%1:%2:%3").arg(sNodeName).arg("ITQ").arg(
            "MI5");
    sessionConnectInfo.sProductUri = UaString("urn:%1:%2").arg("ITQ").arg("MI5");
    sessionConnectInfo.sSessionName = UaString("MI5Session:%1").arg(UaString::number(rand() % 10000));
    sessionConnectInfo.bAutomaticReconnect = m_pConfiguration->getAutomaticReconnect();
    sessionConnectInfo.bRetryInitialConnect = m_pConfiguration->getRetryInitialConnect();

    // Security settings are not initialized - we connect without security for now
    SessionSecurityInfo sessionSecurityInfo;

    printf("\nConnecting to %s\n", m_serverUrl.toUtf8());
    result = m_pSession->connect(
                 m_serverUrl,
                 sessionConnectInfo,
                 sessionSecurityInfo,
                 this);

    if (result.isGood())
    {
        printf("Connect succeeded\n");
    }
    else
    {
        QLOG_ERROR() << "Connect failed with status " << result.toString().toUtf8();
    }

    return result;
}

UaStatus OpcuaGateway::disconnect()
{
    UaStatus result;

    // Default settings like timeout
    ServiceSettings serviceSettings;

    printf("\nDisconnecting ...\n");
    result = m_pSession->disconnect(
                 serviceSettings,
                 OpcUa_True);

    if (result.isGood())
    {
        printf("Disconnect succeeded\n");
    }
    else
    {
        printf("Disconnect failed with status %s\n", result.toString().toUtf8());
    }

    return result;
}

UaStatus OpcuaGateway::browseSimple()
{
    UaStatus result;
    UaNodeId nodeToBrowse;

    // browse from root folder with no limitation of references to return
    nodeToBrowse = UaNodeId(OpcUaId_RootFolder);
    result = browseInternal(nodeToBrowse, 0);
    return result;
}

UaStatus OpcuaGateway::browseContinuationPoint()
{
    UaStatus result;
    UaNodeId nodeToBrowse;

    // browse from Massfolder with max references to return set to 5
    nodeToBrowse = UaNodeId("GVL", 4);
    result = browseInternal(nodeToBrowse, 5);

    return result;
}

UaDataValues OpcuaGateway::read(UaReadValueIds& nodesToRead)
{
    UaStatus          result;
    ServiceSettings   serviceSettings;
    //UaReadValueIds    nodesToRead;
    UaDataValues      values;
    UaDiagnosticInfos diagnosticInfos;

    //// read all nodes from the configuration
    //UaNodeIdArray nodes = m_pConfiguration->getNodesToRead();
    //nodesToRead.create(nodes.length());

    //for (OpcUa_UInt32 i = 0; i < nodes.length(); i++)
    //{
    //    nodesToRead[i].AttributeId = OpcUa_Attributes_Value;
    //    OpcUa_NodeId_CopyTo(&nodes[i], &nodesToRead[i].NodeId);
    //}

    //   printf("\nReading ...\n");
    result = m_pSession->read(
                 serviceSettings,
                 0,
                 OpcUa_TimestampsToReturn_Both,
                 nodesToRead,
                 values,
                 diagnosticInfos);

    if (result.isGood())
    {
        // Read service succeded - check individual status codes
        for (OpcUa_UInt32 i = 0; i < nodesToRead.length(); i++)
        {
            if (OpcUa_IsGood(values[i].StatusCode))
            {
                //printf("Value[%d]: %s\n", i, UaVariant(values[i].Value).toString().toUtf8());
            }
            else
            {
                QLOG_ERROR() << "Read failed for item[%d] with status " <<
                             UaStatus(values[i].StatusCode).toString().toUtf8();
            }
        }
    }
    else
    {
        // Service call failed
        QLOG_ERROR() << "Read failed with status " << result.toString().toUtf8();
    }

    return values;
}

UaStatus OpcuaGateway::write(UaWriteValues& nodesToWrite)
{
    UaStatus            result;
    ServiceSettings     serviceSettings;
    // UaWriteValues       nodesToWrite;
    UaStatusCodeArray   results;
    UaDiagnosticInfos   diagnosticInfos;

    // write all nodes from the configuration
    UaNodeIdArray nodes;// = m_pConfiguration->getNodesToWrite();
    //UaVariantArray values;// = m_pConfiguration->getWriteValues();
    //nodesToWrite.create(nodes.length());

    //for (OpcUa_UInt32 i = 0; i < nodes.length(); i++)
    //{
    //    nodesToWrite[i].AttributeId = OpcUa_Attributes_Value;
    //    OpcUa_NodeId_CopyTo(&nodes[i], &nodesToWrite[i].NodeId);
    //    // set value to write
    //    OpcUa_Variant_CopyTo(&values[i], &nodesToWrite[i].Value.Value);
    //}

    //printf("\nWriting...\n");
    int tryCounter = 0;

    while (tryCounter < 2)
    {

        result = m_pSession->write(
                     serviceSettings,
                     nodesToWrite,
                     results,
                     diagnosticInfos);

        if (result.isGood())
        {
            // Write service succeded - check individual status codes
            for (OpcUa_UInt32 i = 0; i < results.length(); i++)
            {
                if (OpcUa_IsGood(results[i]))
                {
                    //UaVariant tmpValue = nodesToWrite[i].Value.Value;
                    //QLOG_DEBUG() << "Write suceeded for item[" << i << "]: " <<
                    //          nodesToWrite[i].NodeId.Identifier.String.strContent << " - Value: " << tmpValue.toString().toUtf8()
                    //          ;
                    tryCounter = 2;
                }
                else
                {
                    QLOG_ERROR() << "Write failed for item[%d] with status " << UaStatus(
                                     results[i]).toString().toUtf8();
                    QLOG_ERROR() << "Tries left: " << (1 - tryCounter) ;
                    tryCounter++;
                }
            }
        }
        else
        {
            // Service call failed
            QLOG_ERROR() << "Write failed with status " << result.toString().toUtf8();
        }
    }

    return result;
}

UaStatus OpcuaGateway::createSubscription(int subscriptionHandle)
{
    UaStatus result = m_pSubscription->createSubscription(m_pSession, subscriptionHandle);
    return result;
}

UaStatus OpcuaGateway::createSingleMonitoredItem(OpcUa_UInt32 clientHandle,
        OpcUa_UInt32 subscriptionHandle,
        UaNodeIdArray nodesToSubscribe)
{
    UaStatus result = m_pSubscription->createSingleMonitoredItem(clientHandle, subscriptionHandle,
                      nodesToSubscribe);
    return result;
}

UaStatus OpcuaGateway::subscribe(int subscriptionHandle, UaNodeIdArray
                                 nodesToSubscribe)//, int subscriptionID) //TODO: Module adress
{
    UaStatus result;
    //nodesToSubscribe = m_pConfiguration->getNodesToSubscribe();
    //result = m_pSubscription->createSubscription(m_pSession);

    //if (result.isGood())
    //{
    //    result = m_pSubscription->createMonitoredItems(nodesToSubscribe);
    //    //result = m_pSubscription->createMonitoredItems(m_pConfiguration->getNodesToSubscribe());
    //}

    return result;
}

//UaStatus OpcuaGateway::createOneMonitoredItem(subscription ID??

UaStatus OpcuaGateway::unsubscribe()
{
    //TODO
    //return m_pSubscription->deleteSubscription();

    UaStatus result;
    return result;
}


UaStatus OpcuaGateway::browseInternal(const UaNodeId& nodeToBrowse,
                                      OpcUa_UInt32 maxReferencesToReturn)
{
    UaStatus result;

    ServiceSettings serviceSettings;
    BrowseContext browseContext;
    UaByteString continuationPoint;
    UaReferenceDescriptions referenceDescriptions;

    // configure browseContext
    browseContext.browseDirection = OpcUa_BrowseDirection_Forward;
    browseContext.referenceTypeId = OpcUaId_HierarchicalReferences;
    browseContext.includeSubtype = OpcUa_True;
    browseContext.maxReferencesToReturn = maxReferencesToReturn;

    printf("\nBrowsing from Node %s...\n", nodeToBrowse.toXmlString().toUtf8());
    result = m_pSession->browse(
                 serviceSettings,
                 nodeToBrowse,
                 browseContext,
                 continuationPoint,
                 referenceDescriptions);

    if (result.isGood())
    {
        // print results
        printBrowseResults(referenceDescriptions);

        // continue browsing
        while (continuationPoint.length() > 0)
        {
            printf("\nContinuationPoint is set. BrowseNext...\n");
            // browse next
            result = m_pSession->browseNext(
                         serviceSettings,
                         OpcUa_False,
                         continuationPoint,
                         referenceDescriptions);

            if (result.isGood())
            {
                // print results
                printBrowseResults(referenceDescriptions);
            }
            else
            {
                // Service call failed
                printf("BrowseNext failed with status %s\n", result.toString().toUtf8());
            }
        }
    }
    else
    {
        // Service call failed
        printf("Browse failed with status %s\n", result.toString().toUtf8());
    }

    return result;
}

void OpcuaGateway::printBrowseResults(const UaReferenceDescriptions& referenceDescriptions)
{
    OpcUa_UInt32 i;

    for (i = 0; i < referenceDescriptions.length(); i++)
    {
        printf("node: ");
        UaNodeId referenceTypeId(referenceDescriptions[i].ReferenceTypeId);
        printf("[Ref=%s] ", referenceTypeId.toString().toUtf8());
        UaQualifiedName browseName(referenceDescriptions[i].BrowseName);
        printf("%s ( ", browseName.toString().toUtf8());

        if (referenceDescriptions[i].NodeClass & OpcUa_NodeClass_Object) { printf("Object "); }

        if (referenceDescriptions[i].NodeClass & OpcUa_NodeClass_Variable) { printf("Variable "); }

        if (referenceDescriptions[i].NodeClass & OpcUa_NodeClass_Method) { printf("Method "); }

        if (referenceDescriptions[i].NodeClass & OpcUa_NodeClass_ObjectType) { printf("ObjectType "); }

        if (referenceDescriptions[i].NodeClass & OpcUa_NodeClass_VariableType) { printf("VariableType "); }

        if (referenceDescriptions[i].NodeClass & OpcUa_NodeClass_ReferenceType) { printf("ReferenceType "); }

        if (referenceDescriptions[i].NodeClass & OpcUa_NodeClass_DataType) { printf("DataType "); }

        if (referenceDescriptions[i].NodeClass & OpcUa_NodeClass_View) { printf("View "); }

        UaNodeId nodeId(referenceDescriptions[i].NodeId.NodeId);
        printf("[NodeId=%s] ", nodeId.toFullString().toUtf8());
        printf(")\n");
    }
}


void OpcuaGateway::registerModule(int moduleNumber, IModule* pModule)
{
    if (m_moduleList.count(moduleNumber) > 0)
    {
        QLOG_DEBUG() << "Error, module number " << moduleNumber << " is already registered.";
    }
    else
    {
        m_moduleList[moduleNumber] = pModule;
    }
}

void OpcuaGateway::subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
        const UaDataNotifications& dataNotifications,
        const UaDiagnosticInfos&   diagnosticInfos)
{
    if (m_moduleList.count(clientSubscriptionHandle) > 0)
    {
        m_moduleList[clientSubscriptionHandle]->subscriptionDataChange(clientSubscriptionHandle,
                dataNotifications,
                diagnosticInfos);
    }
    else
    {
        QLOG_DEBUG() << "Subscription for module number " << clientSubscriptionHandle <<
                     " received, but module is not present." ;
    }
}