#include <Mi5_ProcessTool/include/TaskModule.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <qcoreapplication.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

TaskModule::TaskModule(OpcuaGateway* pOpcuaGateway, int moduleNumber,
                       std::map<int, IProductionModule*> moduleList, MessageFeeder* pMessageFeeder,
                       IProductionModule* pManual) : m_pManual(pManual)
{
    m_taskCounter = 0;
    m_moduleSkillList.clear();
    m_taskObjects.clear();

    m_pOpcuaGateway = pOpcuaGateway;
    m_moduleNumber = moduleNumber;
    m_moduleList = moduleList;
    m_pMsgFeed = pMessageFeeder;

    m_pOpcuaGateway->registerModule(m_moduleNumber, this);
    QLOG_DEBUG() << "Created TaskModule with module number " << moduleNumber ;
}

TaskModule::~TaskModule()
{
}

void TaskModule::startup()
{
    setupOpcua();
}

void TaskModule::setupOpcua()
{
    UaStatus status;
    status = m_pOpcuaGateway->createSubscription(m_moduleNumber);

    if (!status.isGood())
    {
        QLOG_DEBUG() << "Creation of subscription for module number " << m_moduleNumber << " failed.";
        return;
    }

    createMonitoredItems();
}

void TaskModule::serverReconnected()
{
    setupOpcua();
}

void TaskModule::createMonitoredItems()
{
    int clientHandleNumber;
    UaStatus status;

    for (OpcUa_Int32 i = 0; i < TASKCOUNT; i++)
    {
        nodeIdToSubscribe = "ns=4;s=MI5.ProductionList[";
        nodeIdToSubscribe += UaString::number(i);
        nodeIdToSubscribe += "].";
        nodeIdToSubscribe += "Dummy";
        createNodeStructure();
        status = m_pOpcuaGateway->createSingleMonitoredItem(1000 + i, m_moduleNumber,
                 nodeToSubscribe);
    }

}

void TaskModule::createNodeStructure()
{
    nodeToSubscribe.clear();
    nodeToSubscribe.resize(1);
    UaNodeId::fromXmlString(nodeIdToSubscribe).copyTo(&nodeToSubscribe[0]);
}


void TaskModule::subscriptionDataChange(OpcUa_UInt32 clientSubscriptionHandle,
                                        const UaDataNotifications& dataNotifications,
                                        const UaDiagnosticInfos&   diagnosticInfos)
{
    if (clientSubscriptionHandle == m_moduleNumber)
    {
        OpcUa_ReferenceParameter(diagnosticInfos);

        for (OpcUa_Int32 i = 0; i < dataNotifications.length(); i++)
        {
            // QLOG_DEBUG() << dataNotifications[i].ClientHandle ;

            if (OpcUa_IsGood(dataNotifications[i].Value.StatusCode))
            {
                UaVariant tempValue = dataNotifications[i].Value.Value;
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

bool TaskModule::isTaskDone(int taskStructNumber)
{
    bool val = false;

    UaStatus status;
    UaDataValues returnValues;
    OpcUa_NodeId tmpNodeId;
    UaReadValueIds nodesToRead;
    int readCounter = 0;

    nodesToRead.create(1);
    UaString baseNodeId = "ns=4;s=MI5.ProductionList[";
    baseNodeId += UaString::number(taskStructNumber);
    baseNodeId += "].";

    UaString nodeIdToRead = baseNodeId;
    nodeIdToRead += "State";
    UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    returnValues = m_pOpcuaGateway->read(nodesToRead);

    OpcUa_Int32 tmpReturnVal;
    UaVariant(returnValues[0].Value).toInt32(tmpReturnVal);

    if (tmpReturnVal == TaskDone)
    {
        val = true;
    }

    return val;

}


// Dont look.
void TaskModule::moduleDataChange(const UaDataNotifications& dataNotifications)
{
    for (OpcUa_UInt32 i = 0; i < dataNotifications.length(); i++)
    {
        if (OpcUa_IsGood(dataNotifications[i].Value.StatusCode))
        {
            // Extract value.
            UaVariant tempValue = dataNotifications[i].Value.Value;

            // Some output
            /*printf("  Variable %d value = %s\n", dataNotifications[i].ClientHandle,
                   tempValue.toString().toUtf8());*/

            // INPUT
            OpcUa_Int32 taskNumber = (dataNotifications[i].ClientHandle % 1000) % TASKCOUNT;
            OpcUa_Boolean dummy;
            tempValue.toBool(dummy);

            if ((taskNumber < TASKCOUNT) &&
                (dummy == false) &&
                isTaskDone(
                    taskNumber)) // check, wether this is the right subscription and activation of task and the task has not been processed yet
            {

                if (m_tasklist.count(taskNumber) == 0)
                {
                    m_tasklist[taskNumber].dummy = true;
                    m_tasklist[taskNumber].taskNumberInStructure = taskNumber;

                    UaStatus status = getTaskInformation(taskNumber);

                    if (!status.isGood())
                    {
                        QLOG_DEBUG() << "Error: Couldn't retrieve task information for task number " << taskNumber;
                        return;
                    }

                    QLOG_DEBUG() << "Received new task (#" << taskNumber << "): " <<
                                 m_tasklist[taskNumber].name.toUtf8()
                                 ;
                    m_taskObjects[m_tasklist[taskNumber].taskId] = new Task(m_tasklist[taskNumber], m_moduleList, this,
                            m_pMsgFeed, m_pManual);
                    m_taskObjects[m_tasklist[taskNumber].taskId]->start();
                }

                else
                {
                    //task number already exists
                    QLOG_DEBUG() << "Error, task number " << taskNumber << " already exists in tasklist." ;
                }
            }

            else
            {
                // Subscription for someone else, or dummy changed from false to trues
            }
        }
    }
}

void TaskModule::notifyTaskDone(OpcUa_Int32& taskId, OpcUa_Int32& taskNumber)
{
    if (m_taskObjects.count(taskId) > 0)
    {
        m_tasklist.erase(m_tasklist.find(taskNumber));

        std::map<int, Task*>::iterator it = m_taskObjects.find(taskId);

        if (it != m_taskObjects.end())
        {
            updateTaskState(taskNumber, TaskDone);
            Task* tmp = it->second;
            m_taskObjects.erase(it);
            //delete tmp; // whoops, we have a memory leak here, as the task objects still receives skillStateUpdates
        }
    }
}

void TaskModule::updateTaskStructure(ProductionTask& updatedTask, int skillNumberInTask)
{
    m_tasklist[updatedTask.taskNumberInStructure].skill[skillNumberInTask] =
        updatedTask.skill[skillNumberInTask];
    writeTaskInformation(updatedTask.taskNumberInStructure, skillNumberInTask);
}


void TaskModule::updateTaskState(int taskNumber, OpcUa_Int32 state)
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(1);
    int writeCounter = 0;
    UaVariant tmpValue;

    UaString tmpNodeId = "ns=4;s=MI5.ProductionList[";
    tmpNodeId += UaString::number(taskNumber);
    tmpNodeId += "].State";
    tmpValue.setInt32(state);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    // Write!
    m_pOpcuaGateway->write(nodesToWrite);
}


UaStatus TaskModule::writeTaskInformation(OpcUa_Int32 taskNumber, int skillNumberInTask)
{
    UaStatus status;
    UaWriteValues nodesToWrite;
    nodesToWrite.create(/*1 + */3 /* SKILLCOUNT*/); // Fill in number
    int writeCounter = 0;
    UaVariant tmpValue;

    UaString baseNodeIdToWrite = "ns=4;s=MI5.ProductionList[";
    baseNodeIdToWrite += UaString::number(taskNumber);
    baseNodeIdToWrite += "].";

    updateTaskState(taskNumber, m_tasklist[taskNumber].taskState);

    /*   for (int i = 0; i < SKILLCOUNT; i++)
       {*/
    UaString baseSkillNodeId = baseNodeIdToWrite;
    baseSkillNodeId += "Skill[";
    baseSkillNodeId += UaString::number(skillNumberInTask);
    baseSkillNodeId += "].";

    UaString tmpNodeId = baseSkillNodeId;
    tmpNodeId += "AssignedModuleID";
    tmpValue.setUInt16(m_tasklist[taskNumber].skill[skillNumberInTask].assignedModuleId);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "AssignedModuleName";
    tmpValue.setString(m_tasklist[taskNumber].skill[skillNumberInTask].assignedModuleName);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "AssignedModulePosition";
    tmpValue.setDouble(m_tasklist[taskNumber].skill[skillNumberInTask].assignedModulePosition);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    //}

    // Write!
    m_pOpcuaGateway->write(nodesToWrite);

    return status;
}


UaStatus TaskModule::getTaskInformation(OpcUa_Int32 taskNumber)
{
    UaStatus status;
    UaDataValues returnValues;
    OpcUa_NodeId tmpNodeId;
    UaReadValueIds nodesToRead;
    int readCounter = 0;

    nodesToRead.create(5 + (SKILLCOUNT) * (6 + 10 *
                                           (PARAMETERCOUNT))); // Fill in number, should be 4799
    UaString baseNodeId = "ns=4;s=MI5.ProductionList[";
    baseNodeId += UaString::number(taskNumber);
    baseNodeId += "].";

    UaString nodeIdToRead = baseNodeId;
    nodeIdToRead += "Name";
    UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    nodeIdToRead = baseNodeId;
    nodeIdToRead += "TaskID";
    UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    nodeIdToRead = baseNodeId;
    nodeIdToRead += "RecipeID";
    UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    nodeIdToRead = baseNodeId;
    nodeIdToRead += "Timestamp";
    UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    nodeIdToRead = baseNodeId;
    nodeIdToRead += "State";
    UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    for (int i = 0; i < SKILLCOUNT; i++)
    {
        UaString baseSkillNodeId = baseNodeId;
        baseSkillNodeId += "Skill[";
        baseSkillNodeId += UaString::number(i);
        baseSkillNodeId += "].";

        nodeIdToRead = baseSkillNodeId;
        nodeIdToRead += "AssignedModuleName";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        nodeIdToRead = baseSkillNodeId;
        nodeIdToRead += "AssignedModuleID";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        nodeIdToRead = baseSkillNodeId;
        nodeIdToRead += "AssignedModulePosition";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        nodeIdToRead = baseSkillNodeId;
        nodeIdToRead += "Dummy";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        nodeIdToRead = baseSkillNodeId;
        nodeIdToRead += "ID";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        nodeIdToRead = baseSkillNodeId;
        nodeIdToRead += "Name";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        for (int j = 0; j < PARAMETERCOUNT; j++)
        {
            UaString baseParamNodeId = baseSkillNodeId;
            baseParamNodeId += "Parameter[";
            baseParamNodeId += UaString::number(j);
            baseParamNodeId += "].";

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "Dummy";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "ID";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "Name";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "Unit";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "Required";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "Default";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "MinValue";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "MaxValue";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "Value";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            nodeIdToRead = baseParamNodeId;
            nodeIdToRead += "StringValue";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;
        }
    }

    // Read!
    returnValues = m_pOpcuaGateway->read(nodesToRead);
    readCounter = 0;

    m_tasklist[taskNumber].name = UaVariant(returnValues[readCounter].Value).toString();
    readCounter++;
    UaVariant(returnValues[readCounter].Value).toInt32(m_tasklist[taskNumber].taskId);
    readCounter++;
    UaVariant(returnValues[readCounter].Value).toInt32(m_tasklist[taskNumber].recipeId);
    readCounter++;
    m_tasklist[taskNumber].timestamp = UaVariant(returnValues[readCounter].Value).toString();
    readCounter++;
    UaVariant(returnValues[readCounter].Value).toInt32(m_tasklist[taskNumber].taskState);
    readCounter++;

    for (int i = 0; i < SKILLCOUNT; i++)
    {
        m_tasklist[taskNumber].skill[i].assignedModuleName = UaVariant(
                    returnValues[readCounter].Value).toString();
        readCounter++;
        UaVariant(returnValues[readCounter].Value).toUInt32(
            m_tasklist[taskNumber].skill[i].assignedModuleId);
        readCounter++;
        UaVariant(returnValues[readCounter].Value).toDouble(
            m_tasklist[taskNumber].skill[i].assignedModulePosition);
        readCounter++;
        UaVariant(returnValues[readCounter].Value).toBool(
            m_tasklist[taskNumber].skill[i].dummy);
        readCounter++;
        UaVariant(returnValues[readCounter].Value).toUInt32(
            m_tasklist[taskNumber].skill[i].id);
        readCounter++;
        m_tasklist[taskNumber].skill[i].name = UaVariant(
                returnValues[readCounter].Value).toString();
        readCounter++;

        for (int j = 0; j < PARAMETERCOUNT; j++)
        {
            UaVariant(returnValues[readCounter].Value).toBool(
                m_tasklist[taskNumber].skill[i].parameter[j].dummy);
            readCounter++;
            UaVariant(returnValues[readCounter].Value).toUInt32(
                m_tasklist[taskNumber].skill[i].parameter[j].id);
            readCounter++;
            m_tasklist[taskNumber].skill[i].parameter[j].name = UaVariant(
                        returnValues[readCounter].Value).toString();
            readCounter++;
            m_tasklist[taskNumber].skill[i].parameter[j].unit = UaVariant(
                        returnValues[readCounter].Value).toString();
            readCounter++;
            UaVariant(returnValues[readCounter].Value).toBool(
                m_tasklist[taskNumber].skill[i].parameter[j].required);
            readCounter++;
            UaVariant(returnValues[readCounter].Value).toDouble(
                m_tasklist[taskNumber].skill[i].parameter[j].defaultParameter);
            readCounter++;
            UaVariant(returnValues[readCounter].Value).toDouble(
                m_tasklist[taskNumber].skill[i].parameter[j].minvalue);
            readCounter++;
            UaVariant(returnValues[readCounter].Value).toDouble(
                m_tasklist[taskNumber].skill[i].parameter[j].maxValue);
            readCounter++;
            UaVariant(returnValues[readCounter].Value).toDouble(
                m_tasklist[taskNumber].skill[i].parameter[j].value);
            readCounter++;
            m_tasklist[taskNumber].skill[i].parameter[j].stringValue = UaVariant(
                        returnValues[readCounter].Value).toString();
            readCounter++;
        }
    }


    return status;
}

std::vector<skillModuleList> TaskModule::getSkillList()
{
    if (m_moduleSkillList.size() == 0)
    {
        buildSkillList();
    }

    return m_moduleSkillList;
}

void TaskModule::buildSkillList()
{

    for (std::map<int, IProductionModule*>::iterator it = m_moduleList.begin();
         it != m_moduleList.end(); ++it)
    {
        skillModuleList tmpList;

        std::map<int, int> tmpMap = it->second->getSkills();
        tmpList.moduleNumber = it->first;

        for (std::map<int, int>::iterator it2 = tmpMap.begin(); it2 != tmpMap.end(); it2++)
        {
            tmpList.skillId = it2->first;
            tmpList.skillPos = it2->second;
            m_moduleSkillList.push_back(tmpList);
        }
    }
}