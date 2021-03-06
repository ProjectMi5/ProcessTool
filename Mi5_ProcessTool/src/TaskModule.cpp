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

    m_abortionTimer = new QTimer(this);
    m_abortionTimer->setSingleShot(true);
    // TODO: Set interval here and not in the start()-call.
    connect(m_abortionTimer, SIGNAL(timeout()), this, SLOT(abortionTimerTriggered()));

    // TODO: If this works, set the timer to singleShot, and delete the stop()-call.
    m_taskUpdateCounter = new QTimer(this);
    m_taskUpdateCounter->setInterval(2000);
    connect(m_taskUpdateCounter, SIGNAL(timeout()), this, SLOT(checkTaskStates()));

    m_pOpcuaGateway = pOpcuaGateway;
    m_moduleNumber = moduleNumber;
    m_moduleList = moduleList;
    m_pMsgFeed = pMessageFeeder;

    m_pOpcuaGateway->registerModule(m_moduleNumber, this);
    QLOG_DEBUG() << "Created TaskModule with module number " << moduleNumber;

    moveToThread(&m_thread);
    m_thread.start();
}

TaskModule::~TaskModule()
{
}

void TaskModule::abortionTimerTriggered()
{
    if (m_taskObjects.count(m_tasklist[m_taskNumberToAbort].taskId) == 0)
    {
        return;
    }

    m_taskObjects[m_tasklist[m_taskNumberToAbort].taskId]->triggerAbortTaskTimeout();
}


void TaskModule::startup()
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "startup", Qt::QueuedConnection);
        return;
    }

    setupOpcua();
    m_taskUpdateCounter->start();
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

    //for (OpcUa_Int32 i = 0; i < TASKCOUNT; i++)
    //{
    //    nodeIdToSubscribe = "ns=4;s=MI5.ProductionList[";
    //    nodeIdToSubscribe += UaString::number(i);
    //    nodeIdToSubscribe += "].";
    //    nodeIdToSubscribe += "Dummy";
    //    createNodeStructure();
    //    status = m_pOpcuaGateway->createSingleMonitoredItem(1000 + i, m_moduleNumber,
    //             nodeToSubscribe);

    //    nodeIdToSubscribe = "ns=4;s=MI5.ProductionList[";
    //    nodeIdToSubscribe += UaString::number(i);
    //    nodeIdToSubscribe += "].";
    //    nodeIdToSubscribe += "AbortTask";
    //    createNodeStructure();
    //    status = m_pOpcuaGateway->createSingleMonitoredItem(666 + i, m_moduleNumber,
    //             nodeToSubscribe);

    //}

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
            // QLOG_DEBUG() << dataNotifications[i].ClientHandle;

            if (OpcUa_IsGood(dataNotifications[i].Value.StatusCode))
            {
                UaVariant tempValue = dataNotifications[i].Value.Value;
            }
            else
            {
                UaStatus itemError(dataNotifications[i].Value.StatusCode);
                QLOG_ERROR() << "Variable " << dataNotifications[i].ClientHandle << " failed with status " <<
                             itemError.toString().toUtf8();
            }
        }

        moduleDataChange(dataNotifications);
    }
    else
    {
        QLOG_ERROR() << "Module number " << m_moduleNumber << " received subscription for " <<
                     clientSubscriptionHandle << "." ;
    }
}

bool TaskModule::isTaskDone(int taskStructNumber)
{
    bool val = false;

    /*UaStatus status;
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
    }*/

    if (m_tasklist[taskStructNumber].taskState == TaskDone)
    {
        val = true;
    }

    return val;

}

void TaskModule::abortTheTask(int taskNumber)
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "abortTheTask", Qt::QueuedConnection, Q_ARG(int, taskNumber));
        return;
    }

    if (m_tasklist.count(taskNumber) == 0)
    {
        return;
    }

    if (m_taskObjects[m_tasklist[taskNumber].taskId] == 0)
    {
        return;
    }

    if (!(m_abortionTimer->isActive()))
    {
        m_taskObjects[m_tasklist[taskNumber].taskId]->abortTask();
        m_abortionTimer->start(20000);
        m_taskNumberToAbort = taskNumber;
        QLOG_DEBUG() << "Started: Abortion timer for task #" << taskNumber << ".";
    }
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
            OpcUa_Int32 taskNumber;// = (dataNotifications[i].ClientHandle % 1000) % TASKCOUNT;

            for (int j = 0; j < SKILLCOUNT; j++)
            {
                if (dataNotifications[i].ClientHandle % (666 + j) == 0)
                {
                    OpcUa_Boolean abortTask = false;
                    tempValue.toBool(abortTask);
                    taskNumber = j;

                    if (abortTask == (OpcUa_Boolean)true)
                    {
                        abortTheTask(taskNumber);
                    }

                    break;
                }
                else if (dataNotifications[i].ClientHandle % (1000 + j) == 0)
                {
                    taskNumber = j;
                    OpcUa_Boolean dummy;
                    tempValue.toBool(dummy);

                    if ((taskNumber < TASKCOUNT) &&
                        (dummy == false) &&
                        !isTaskDone(
                            taskNumber)) // check, wether this is the right subscription and activation of task and the task has not been processed yet
                    {

                        if (m_tasklist.count(taskNumber) == 0)
                        {
                            m_tasklist[taskNumber].dummy = true;
                            m_tasklist[taskNumber].taskNumberInStructure = taskNumber;

                            int status = getTaskInformation(taskNumber);

                            if ((-1) == status)
                            {
                                QLOG_ERROR() << "Error: Couldn't retrieve task information for task number " << taskNumber;
                                return;
                            }

                            QLOG_DEBUG() << "Received new task (#" << taskNumber << ", ID #" << m_tasklist[taskNumber].taskId <<
                                         "): " <<
                                         m_tasklist[taskNumber].name.toUtf8()
                                         ;
                            m_taskObjects[m_tasklist[taskNumber].taskId] = new Task(m_tasklist[taskNumber], m_moduleList, this,
                                    m_pMsgFeed, m_pManual);
                            m_taskObjects[m_tasklist[taskNumber].taskId]->start();
                            break;
                        }

                        else
                        {
                            //task number already exists
                            QLOG_ERROR() << "Error, task number " << taskNumber << " already exists in tasklist." ;
                            break;
                        }
                    }

                    else
                    {
                        // Subscription for someone else, or dummy changed from false to true
                    }
                }
            }
        }
    }
}

void TaskModule::notifyTaskDone(OpcUa_Int32 taskId, OpcUa_Int32 taskNumber, OpcUa_Int32 state)
{
    if (thread() != QThread::currentThread())
    {
        qRegisterMetaType<OpcUa_Int32>("OpcUa_Int32");
        QMetaObject::invokeMethod(this, "notifyTaskDone", Qt::QueuedConnection, Q_ARG(OpcUa_Int32,
                                  taskId), Q_ARG(OpcUa_Int32, taskNumber), Q_ARG(OpcUa_Int32, state));
        return;
    }

    if (m_taskObjects.count(taskId) > 0)
    {
        m_tasklist.erase(m_tasklist.find(taskNumber));
        m_activeTasks.erase(std::find(m_activeTasks.begin(), m_activeTasks.end(), taskNumber));
        std::map<int, Task*>::iterator it = m_taskObjects.find(taskId);

        if (it != m_taskObjects.end())
        {
            m_abortionTimer->stop();
            updateTaskState(taskNumber, state);
            Task* tmp = it->second;
            m_taskObjects.erase(it);
            return;
            delete tmp;
        }
    }

    QLOG_ERROR() << "Found no task with task #" << taskNumber << "(ID #" << taskId << ") to delete.";
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

    m_pOpcuaGateway->write(nodesToWrite);

    if ((TaskDone == state) || (TaskError == state))
    {
        nodesToWrite.clear();
        nodesToWrite.create(1);
        writeCounter = 0;

        tmpNodeId = "ns=4;s=MI5.ProductionList[";
        tmpNodeId += UaString::number(taskNumber);
        tmpNodeId += "].Dummy";
        tmpValue.setBool(true);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();
        m_pOpcuaGateway->write(nodesToWrite);
    }

    // Write!

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


int TaskModule::getTaskInformation(OpcUa_Int32 taskNumber)
{
    //Slim down vars to read.
    int tmpSkillCount = SKILLCOUNT;
    int tmpParameterCount = PARAMETERCOUNT;

    int status = -1;
    UaDataValues returnValues;
    OpcUa_NodeId tmpNodeId;
    UaReadValueIds nodesToRead;
    int readCounter = 0;

    /*nodesToRead.create(5 + (tmpSkillCount) * (7 + 10 *
                       (tmpParameterCount))); // Fill in number, should be 4799*/
    nodesToRead.create(5 + tmpSkillCount * (7 + 2 * tmpParameterCount));

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

    for (int i = 0; i < tmpSkillCount; i++)
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

        nodeIdToRead = baseSkillNodeId;
        nodeIdToRead += "State";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        for (int j = 0; j < tmpParameterCount; j++)
        {
            UaString baseParamNodeId = baseSkillNodeId;
            baseParamNodeId += "Parameter[";
            baseParamNodeId += UaString::number(j);
            baseParamNodeId += "].";

            //nodeIdToRead = baseParamNodeId;
            //nodeIdToRead += "Dummy";
            //UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            //nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            //readCounter++;

            //nodeIdToRead = baseParamNodeId;
            //nodeIdToRead += "ID";
            //UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            //nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            //readCounter++;

            //nodeIdToRead = baseParamNodeId;
            //nodeIdToRead += "Name";
            //UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            //nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            //readCounter++;

            //nodeIdToRead = baseParamNodeId;
            //nodeIdToRead += "Unit";
            //UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            //nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            //readCounter++;

            //nodeIdToRead = baseParamNodeId;
            //nodeIdToRead += "Required";
            //UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            //nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            //readCounter++;

            //nodeIdToRead = baseParamNodeId;
            //nodeIdToRead += "Default";
            //UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            //nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            //readCounter++;

            //nodeIdToRead = baseParamNodeId;
            //nodeIdToRead += "MinValue";
            //UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            //nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            //readCounter++;

            //nodeIdToRead = baseParamNodeId;
            //nodeIdToRead += "MaxValue";
            //UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            //nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            //readCounter++;

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

    if (returnValues.length() != readCounter)
    {
        status = -1;
        return status;
    }

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

    for (int i = 0; i < tmpSkillCount; i++)
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
        UaVariant(returnValues[readCounter].Value).toInt32(
            m_tasklist[taskNumber].skill[i].state);
        readCounter++;

        for (int j = 0; j < tmpParameterCount; j++)
        {
            //UaVariant(returnValues[readCounter].Value).toBool(
            //    m_tasklist[taskNumber].skill[i].parameter[j].dummy);
            //readCounter++;
            //UaVariant(returnValues[readCounter].Value).toUInt32(
            //    m_tasklist[taskNumber].skill[i].parameter[j].id);
            //readCounter++;
            //m_tasklist[taskNumber].skill[i].parameter[j].name = UaVariant(
            //            returnValues[readCounter].Value).toString();
            //readCounter++;
            //m_tasklist[taskNumber].skill[i].parameter[j].unit = UaVariant(
            //            returnValues[readCounter].Value).toString();
            //readCounter++;
            //UaVariant(returnValues[readCounter].Value).toBool(
            //    m_tasklist[taskNumber].skill[i].parameter[j].required);
            //readCounter++;
            //UaVariant(returnValues[readCounter].Value).toDouble(
            //    m_tasklist[taskNumber].skill[i].parameter[j].defaultParameter);
            //readCounter++;
            //UaVariant(returnValues[readCounter].Value).toDouble(
            //    m_tasklist[taskNumber].skill[i].parameter[j].minvalue);
            //readCounter++;
            //UaVariant(returnValues[readCounter].Value).toDouble(
            //    m_tasklist[taskNumber].skill[i].parameter[j].maxValue);
            //readCounter++;
            UaVariant(returnValues[readCounter].Value).toDouble(
                m_tasklist[taskNumber].skill[i].parameter[j].value);
            readCounter++;
            m_tasklist[taskNumber].skill[i].parameter[j].stringValue = UaVariant(
                        returnValues[readCounter].Value).toString();
            readCounter++;
        }
    }

    status = 1;
    return status;
}

std::vector<skillModuleList> TaskModule::getSkillList()
{
    //if (m_moduleSkillList.size() == 0)
    //{
    buildSkillList(); // Possible overhead.
    //}

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

void TaskModule::updateSkillState(int taskNumber, int skillNumber, OpcUa_Int32 state)
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(1);
    int writeCounter = 0;
    UaVariant tmpValue;

    UaString tmpNodeId = "ns=4;s=MI5.ProductionList[";
    tmpNodeId += UaString::number(taskNumber);
    tmpNodeId += "].Skill[";
    tmpNodeId += UaString::number(skillNumber);
    tmpNodeId += "].State";
    tmpValue.setInt32(state);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    m_pOpcuaGateway->write(nodesToWrite);
}

void TaskModule::checkTaskStates()
{
    // Stop the timer
    m_taskUpdateCounter->stop();

    UaDataValues returnValues;
    OpcUa_NodeId tmpNodeId;
    UaReadValueIds nodesToRead;
    int readCounter = 0;

    nodesToRead.create(3 * TASKCOUNT);
    UaString baseNodeId = "ns=4;s=MI5.ProductionList[";


    for (int i = 0; i < TASKCOUNT; i++)
    {
        UaString tmpNodeId = baseNodeId;
        tmpNodeId += UaString::number(i);
        tmpNodeId += "].";

        UaString nodeIdToRead = tmpNodeId;
        nodeIdToRead += "Dummy";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        nodeIdToRead = tmpNodeId;
        nodeIdToRead += "State";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;

        nodeIdToRead = tmpNodeId;
        nodeIdToRead += "AbortTask";
        UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
        nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
        readCounter++;
    }

    // Read!
    returnValues = m_pOpcuaGateway->read(nodesToRead);

    if (returnValues.length() != readCounter)
    {
        return;
    }

    readCounter = 0;

    for (int i = 0; i < TASKCOUNT; i++)
    {
        UaVariant(returnValues[readCounter].Value).toBool(m_tasklist[i].dummy);
        readCounter++;
        UaVariant(returnValues[readCounter].Value).toInt32(m_tasklist[i].taskState);
        readCounter++;
        UaVariant(returnValues[readCounter].Value).toBool(m_tasklist[i].abortTask);
        readCounter++;
    }

    evalTaskList();
}

void TaskModule::evalTaskList()
{
    for (int taskNumber = 0; taskNumber < TASKCOUNT; taskNumber++)
    {
        if (m_tasklist[taskNumber].abortTask == (OpcUa_Boolean)true)
        {
            abortTheTask(taskNumber);
        }
        else if (m_tasklist[taskNumber].dummy == false && !isTaskDone(taskNumber))
        {
            if (std::find(m_activeTasks.begin(), m_activeTasks.end(), taskNumber) == m_activeTasks.end())
            {
                m_activeTasks.push_back(taskNumber);
                // tmp debug
                QLOG_DEBUG() << "Added task #" << taskNumber << " to activeTasks";
                //
                m_tasklist[taskNumber].dummy = true;
                m_tasklist[taskNumber].taskNumberInStructure = taskNumber;

                int status = getTaskInformation(taskNumber);

                while ((1) != status)
                {
                    status = getTaskInformation(taskNumber);
                    QLOG_ERROR() << "Error: Couldn't retrieve task information for task number " << taskNumber;
                    return;
                }

                // tmp debug
                QLOG_DEBUG() << "Apparently, received task information for task #" << taskNumber;
                //
                QLOG_DEBUG() << "Received new task (#" << taskNumber << ", ID #" << m_tasklist[taskNumber].taskId <<
                             "): " <<
                             m_tasklist[taskNumber].name.toUtf8();
                m_taskObjects[m_tasklist[taskNumber].taskId] = new Task(m_tasklist[taskNumber], m_moduleList, this,
                        m_pMsgFeed, m_pManual);
                m_taskObjects[m_tasklist[taskNumber].taskId]->start();
            }

            else
            {
                //task number already exists
                //QLOG_ERROR() << "Error, task number " << taskNumber << " already exists in tasklist." ;
            }
        }
    }

    // Restart the timer.
    m_taskUpdateCounter->start();
}