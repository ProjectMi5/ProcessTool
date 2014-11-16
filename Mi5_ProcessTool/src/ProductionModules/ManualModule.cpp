#include <Mi5_ProcessTool/include/ProductionModules/ManualModule.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

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
        QLOG_DEBUG() << "Creation of subscription for module number " << m_moduleNumber << " failed.";
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
            // QLOG_DEBUG() << dataNotifications[i].ClientHandle ;

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
        QLOG_DEBUG() << "Module number " << m_moduleNumber << " received subscription for " <<
                     clientSubscriptionHandle << "." ;
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

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Position";
    createNodeStructure();
    status = m_pGateway->createSingleMonitoredItem(105, m_moduleNumber,
             nodeToSubscribe);
}

void ManualModule::write()
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(4 + PARAMETERCOUNT * (5)); // Fill in number
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
    tmpValue.setUInt16(data.iSkillId);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "TaskID";
    tmpValue.setUInt16(data.iTaskId);
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
        tmpValue.setUInt16(data.iParameter[j].id);
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
                skillStateChanged(0, 0);
            }
            else if (dataNotifications[i].ClientHandle == 101)
            {
                tempValue.toBool(data.oDone);
                skillStateChanged(0, 0);
            }
            else if (dataNotifications[i].ClientHandle == 102)
            {
                tempValue.toBool(data.oError);
                skillStateChanged(0, 0);
            }
            else if (dataNotifications[i].ClientHandle == 103)
            {
                tempValue.toUInt16(data.oErrorId);
            }
            else if (dataNotifications[i].ClientHandle == 104)
            {
                tempValue.toBool(data.oReady);
                skillStateChanged(0, 0);
            }
            else if (dataNotifications[i].ClientHandle == 105)
            {
                tempValue.toDouble(data.oPosition);
            }
        }
    }
}

void ManualModule::skillStateChanged(int skillPos, int state)
{
    if (m_skillRegistrationList.count(skillPos) > 0) //task registered for this skillpos
    {
        ISkillRegistration* pTask = m_skillRegistrationList[skillPos];

        if (data.oBusy)
        {
            pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEBUSY);
        }

        else if (data.oDone)
        {
            pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEDONE);
        }

        else if (data.oError)
        {
            pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEERROR);
        }

        else if (data.oReady)
        {
            pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEREADY);
        }
    }
    else // no task registered for this skill position
    {
    }
}

void ManualModule::createNodeStructure()
{
    nodeToSubscribe.clear();
    nodeToSubscribe.resize(1);
    UaNodeId::fromXmlString(nodeIdToSubscribe).copyTo(&nodeToSubscribe[0]);
}


std::map<int, int> ManualModule::getSkills()
{
    std::map<int, int> tmpMap;
    tmpMap[MANUALUNIVERSALSKILL] = 0;

    return tmpMap;
}

int ManualModule::checkSkillState(int& skillId)
{
    int returnVal = -1;

    if (skillId != MANUALUNIVERSALSKILL)
    {
        // error
    }
    else
    {
        if (data.oReady)
        {
            returnVal = SKILLMODULEREADY;
        }
        else if (data.oBusy)
        {
            returnVal =  SKILLMODULEBUSY;
        }
        else if (data.oDone)
        {
            returnVal = SKILLMODULEDONE;
        }
        else if (data.oError)
        {
            returnVal = SKILLMODULEERROR;
        }
    }

    return returnVal;
}

bool ManualModule::checkSkillReadyState(int& skillId)
{
    bool tmpBool = false;

    if (skillId == MANUALUNIVERSALSKILL)
    {
        tmpBool = data.oReady;
    }

    return tmpBool;
}

int ManualModule::translateSkillIdToSkillPos(int skillId)
{
    if (skillId == MANUALUNIVERSALSKILL)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void ManualModule::assignSkill(int& taskId, Skill skill, int& skillPos)
{
    //used?!
}

void ManualModule::executeSkill(int& skillPos, ParameterInputArray& paramInput)
{
    for (int i = 0; i < PARAMETERCOUNT; i++)
    {
        data.iParameter[i].stringValue = paramInput.paramInput[i].string;
        data.iParameter[i].value = paramInput.paramInput[i].value;
    }

    data.iExecute = true;
    QLOG_DEBUG() << "Executing skill position #" << skillPos << " at module number " << m_moduleNumber
                 << " (ManualModule)";
    write();
}

void ManualModule::deregisterTaskForSkill(int& skillPos)
{
    if (m_skillRegistrationList.count(skillPos) > 0)
    {
        m_skillRegistrationList.erase(m_skillRegistrationList.find(skillPos));

        data.iExecute = false;

        for (int i = 0; i < PARAMETERCOUNT; i++)
        {
            data.iParameter[i].value = 0;
            data.iParameter[i].stringValue  = UaString("");
            data.iParameter[i].name = UaString("");
            data.iParameter[i].unit = UaString("");
            data.iParameter[i].id = 0;
        }

        write();
    }

    else
    {
        QLOG_DEBUG() << "Module number " << m_moduleNumber <<
                     ": Received deregistration request for not registered skillpos " << skillPos;
    }
}

UaString ManualModule::getSkillName(int& skillPos)
{
    return UaString("ManualModule - UniversalSkill");
}

UaString ManualModule::getModuleName()
{
    return UaString("ManualModule");
}

int ManualModule::getModulePosition()
{
    return data.oPosition;
}

int ManualModule::registerTaskForSkill(ISkillRegistration* pTask, int skillPos)
{
    int returnVal = -1;

    if (m_skillRegistrationList.count(skillPos) > 0)
    {
        QLOG_DEBUG() << "Skill at position " << skillPos << " is already registered with task id " <<
                     m_skillRegistrationList[skillPos]->getTaskId() ;
    }
    else
    {
        m_skillRegistrationList[skillPos] = pTask;
        returnVal = 0;
    }

    return returnVal;
}

// Methods for connection test: not implemented yet.
void ManualModule::writeConnectionTestInput(bool input)
{
    //
}

bool ManualModule::checkConnectionTestOutput()
{
    return false;
}

void ManualModule::moduleDisconnected()
{
    //
}

bool ManualModule::isBlocked()
{
    return false;
}

bool ManualModule::isReserved()
{
    return false;
}

int ManualModule::translateSkillPosToSkillId(int skillPos)
{
    return 0;
    //
}
