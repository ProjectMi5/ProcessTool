#include <Mi5_ProcessTool/include/ProductionModules/ManualModule.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>

ManualModule::ManualModule(OpcuaGateway* pOpcuaGateway, int moduleNumber,
                           MessageFeeder* pMessageFeeder) : m_moduleNumber(moduleNumber), m_pGateway(pOpcuaGateway)
{
    pOpcuaGateway->registerModule(m_moduleNumber, this);
}

ManualModule::~ManualModule()
{

}


void ManualModule::serverReconnected()
{

}

void ManualModule::startup()
{
    setupOpcua();
}

void ManualModule::setupOpcua()
{
    UaStatus status;
    status = m_pGateway->createSubscription(m_moduleNumber);

    if (!status.isGood())
    {
        std::cout << "Creation of subscription for module number " << m_moduleNumber << " failed." <<
                  std::endl;
        return;
    }

    createMonitoredItems();
}

void ManualModule::subscriptionDataChange(OpcUa_UInt32 clientSubscriptionHandle,
        const UaDataNotifications& dataNotifications, const UaDiagnosticInfos& diagnosticInfos)
{
    if (clientSubscriptionHandle == m_moduleNumber)
    {
        OpcUa_ReferenceParameter(diagnosticInfos);
        OpcUa_UInt32 i = 0;

        for (i = 0; i < dataNotifications.length(); i++)
        {
            // std::cout << dataNotifications[i].ClientHandle << std::endl;

            if (OpcUa_IsGood(dataNotifications[i].Value.StatusCode))
            {
                UaVariant tempValue = dataNotifications[i].Value.Value;
                /* printf("DataChange Module # %i: Variable %d value = %s\n", m_moduleNumber ,
                        dataNotifications[i].ClientHandle,
                        tempValue.toString().toUtf8());*/
            }
            else
            {
                UaStatus itemError(dataNotifications[i].Value.StatusCode);
                printf("  Variable %d failed with status %s\n", dataNotifications[i].ClientHandle,
                       itemError.toString().toUtf8());
            }
        }

        moduleDataChange(dataNotifications);
    }
    else
    {
        std::cout << "Module number " << m_moduleNumber << " received subscription for " <<
                  clientSubscriptionHandle << "." << std::endl;
    }
}

void ManualModule::createMonitoredItems()
{
    int clientHandleNumber;
    UaStatus status;
    // Prepare
    UaString baseNodeId = "ns=4;s=MI5.Module";
    baseNodeId += UaString::number(m_moduleNumber);
    baseNodeId += "Manual.";

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Busy";
    createNodeStructure();
    status = m_pGateway->createSingleMonitoredItem(100, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Done";
    createNodeStructure();
    status = m_pGateway->createSingleMonitoredItem(101, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Error";
    createNodeStructure();
    status = m_pGateway->createSingleMonitoredItem(102, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "ErrorID";
    createNodeStructure();
    status = m_pGateway->createSingleMonitoredItem(103, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Ready";
    createNodeStructure();
    status = m_pGateway->createSingleMonitoredItem(104, m_moduleNumber,
             nodeToSubscribe);
}

void ManualModule::write()
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(5 + 1 * (1    + PARAMETERCOUNT * 1)); // Fill in number
    int writeCounter = 0;
    UaVariant tmpValue;

    UaString baseNodeIdToWrite = "ns=4;s=MI5.Module";
    baseNodeIdToWrite += UaString::number(m_moduleNumber);
    baseNodeIdToWrite += "Manual.";


    UaString tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "Execute";
    tmpValue.setBool(data.iExecute);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "SkillDescription";
    tmpValue.setString(data.iSkilldescription);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "SkillID";
    tmpValue.setInt32(data.iSkillId);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "TaskID";
    tmpValue.setInt32(data.iTaskId);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();


    for (int j = 0; j < PARAMETERCOUNT; j++)
    {
        UaString baseParamNodeId = baseNodeIdToWrite;
        baseParamNodeId += "Parameter[";
        baseParamNodeId += UaString::number(j);
        baseParamNodeId += "].";

        tmpNodeId = baseParamNodeId;
        tmpNodeId += "ID";
        tmpValue.setInt32(data.iParameter[j].id);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();

        tmpNodeId = baseParamNodeId;
        tmpNodeId += "Value";
        tmpValue.setDouble(data.iParameter[j].value);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();

        tmpNodeId = baseParamNodeId;
        tmpNodeId += "Name";
        tmpValue.setString(data.iParameter[j].name);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();

        tmpNodeId = baseParamNodeId;
        tmpNodeId += "StringValue";
        tmpValue.setString(data.iParameter[j].stringValue);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();

        tmpNodeId = baseParamNodeId;
        tmpNodeId += "Unit";
        tmpValue.setString(data.iParameter[j].unit);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();
    }

    // Write!
    m_pGateway->write(nodesToWrite);
}

void ManualModule::moduleDataChange(const UaDataNotifications& dataNotifications)
{
    for (OpcUa_UInt32 i = 0; i < dataNotifications.length(); i++)
    {
        if (OpcUa_IsGood(dataNotifications[i].Value.StatusCode))
        {
            // Extract value.
            UaVariant tempValue = dataNotifications[i].Value.Value;

            if (dataNotifications[i].ClientHandle == 100)
            {
                tempValue.toBool(data.oBusy);
            }
            else if (dataNotifications[i].ClientHandle == 101)
            {
                tempValue.toBool(data.oDone);
            }
            else if (dataNotifications[i].ClientHandle == 102)
            {
                tempValue.toBool(data.oError);
            }
            else if (dataNotifications[i].ClientHandle == 103)
            {
                tempValue.toInt32(data.oErrorId);
            }
            else if (dataNotifications[i].ClientHandle == 104)
            {
                tempValue.toBool(data.oReady);
            }
        }
    }
}

void ManualModule::createNodeStructure()
{
    nodeToSubscribe.clear();
    nodeToSubscribe.resize(1);
    UaNodeId::fromXmlString(nodeIdToSubscribe).copyTo(&nodeToSubscribe[0]);
}
