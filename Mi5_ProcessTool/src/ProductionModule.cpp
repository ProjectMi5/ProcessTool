#include <Mi5_ProcessTool/include/ProductionModule.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <Mi5_ProcessTool/include/MaintenanceHelper.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <Mi5_ProcessTool/include/InitManager.h>

ProductionModule::ProductionModule(OpcuaGateway* pOpcuaGateway, int moduleNumber,
                                   MessageFeeder* pMessageFeeder, MaintenanceHelper* pHelper,
                                   InitManager* pInitManager, int skillCount) : m_disconnected(false),
    m_pMaintenanceHelper(pHelper), m_pInitManager(pInitManager), m_skillCount(skillCount)
{
    m_moduleSkillList.clear();
    m_skillRegistrationList.clear();

    m_pOpcuaGateway = pOpcuaGateway;
    m_moduleNumber = moduleNumber;
    m_pMsgFeed = pMessageFeeder;

    m_enableConnectionTest = false;


    m_pOpcuaGateway->registerModule(m_moduleNumber, this);

    m_connectionTestTimer = new ConnectionTestTimer(this);
    connect(this, SIGNAL(errorOccured()), this, SLOT(evaluateError()), Qt::QueuedConnection);

    moveToThread(&m_thread);
    m_thread.start();
}

ProductionModule::~ProductionModule()
{
    m_pOpcuaGateway->disconnect();
}

void ProductionModule::startup()
{
    setupOpcua();

    // changeModuleMode(ModuleModeAuto);

    if (((m_moduleNumber >= MODULENUMBERXTSMIN) && (m_moduleNumber <= MODULENUMBERXTSMAX)) ||
        m_moduleNumber == INPUTMODULE || m_moduleNumber == OUTPUTMODULE)
    {
        // Dont init the XTS or the In-/Output modules..
    }
    else if (m_pInitManager != NULL) // Dont init
    {
        m_pInitManager->enqueueForInit(m_moduleNumber);
    }

    if (m_enableConnectionTest)
    {
        m_connectionTestTimer->startUp();
    }
}

void ProductionModule::setupOpcua() // TODO: Implement this in the Init, Task etc. modules, too.
{
    UaStatus status;

    m_baseNodeId = m_pOpcuaGateway->buildBaseNodeId(m_moduleNumber);

    status = m_pOpcuaGateway->createSubscription(m_moduleNumber);


    if (!status.isGood())
    {
        QLOG_DEBUG() << "Creation of subscription for module number " << m_moduleNumber << " failed.";
        return;
    }

    createMonitoredItems();
}

int ProductionModule::translateSkillIdToSkillPos(int skillId)
{
    int returnVal = -1;

    if (m_moduleSkillList.count(skillId) > 0)
    {
        returnVal = m_moduleSkillList[skillId];
    }

    return returnVal;
}


int ProductionModule::translateSkillPosToSkillId(int skillPos)
{
    int returnVal = -1;

    for (std::map<int, int>::iterator it = m_moduleSkillList.begin(); it != m_moduleSkillList.end();
         it++)
    {
        if (it->second == skillPos)
        {
            returnVal = it->first;
            break;
        }
    }

    return returnVal;
}


std::map<int, int> ProductionModule::getSkills()
{
    if (m_disconnected)
    {
        std::map<int, int> emptyList;
        return emptyList;
    }

    buildSkillList();
    return m_moduleSkillList;
}

void ProductionModule::moduleConnectionStatusChanged(int state)
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "moduleConnectionStatusChanged", Qt::QueuedConnection, Q_ARG(int,
                                  state));
        return;
    }

    UaString message("Module ");
    message += getModuleName();

    if (state == ModuleConnectionConnected)
    {
        switch (m_disconnected)
        {
        case true:
            message += " reconnected.";
            startup();
            m_disconnected = false;
            break;

        case false:
            message += " connected.";
            break;
        }

        // todo: create subscriptions, if reconnect.
    }
    // Modul disconnected
    else if (state == ModuleConnectionDisconnected && !m_disconnected)
    {
        m_disconnected = true;
        message += " disconnected.";

        m_pOpcuaGateway->deleteSubscription(m_moduleNumber);
        QLOG_DEBUG() << "Erased subscriptions.";

        if (m_skillRegistrationList.size() > 0)
        {
            for (std::map<int, ISkillRegistration*>::iterator it = m_skillRegistrationList.begin();
                 it != m_skillRegistrationList.end(); it++)
            {
                it->second->skillStateChanged(m_moduleNumber, it->first, SKILLMODULEERROR);
            }
        }
    }
    else
    {
        // .. wait, what?
        return;
    }

    //m_pMsgFeed->write(message, msgError);
    QLOG_ERROR() << message.toUtf8();
}

void ProductionModule::assignSkill(int& taskId, Skill skill, int& skillPos)
{
    for (int i = 0; i < PARAMETERCOUNT; i++)
    {
        input.skillInput[skillPos].parameterInput[i].value = skill.parameter[i].value;
    }

    writeModuleInput();
}

void ProductionModule::executeSkill(int& skillPos, ParameterInputArray& paramInput)
{
    for (int i = 0; i < PARAMETERCOUNT; i++)
    {
        input.skillInput[skillPos].parameterInput[i] = paramInput.paramInput[i];
    }

    input.skillInput[skillPos].execute = true;
    QLOG_DEBUG() << "Executing skill position #" << skillPos << " at module number " << m_moduleNumber
                 << " (" << output.name.toUtf8() << ")";
    writeSkillInput(skillPos);

    checkMoverState(skillPos);
}

void ProductionModule::deregisterTaskForSkill(int& skillPos)
{
    if (m_skillRegistrationList.count(skillPos) > 0)
    {
        m_skillRegistrationList.erase(m_skillRegistrationList.find(skillPos));

        input.skillInput[skillPos].execute = false;

        for (int i = 0; i < PARAMETERCOUNT; i++)
        {
            input.skillInput[skillPos].parameterInput[i].value = 0;
            input.skillInput[skillPos].parameterInput[i].string = UaString("");
        }

        writeSkillInput(skillPos);

        if (m_skillStatePollerList.count(skillPos) > 0)
        {
            m_skillStatePollerList[skillPos]->deleteLater();
            m_skillStatePollerList.erase(m_skillStatePollerList.find(skillPos));
        }
    }

    else
    {
        QLOG_ERROR() << "Module number " << m_moduleNumber <<
                     ": Received deregistration request for unknown skillpos " << skillPos;
    }
}

double ProductionModule::getModulePosition()
{
    double returnVal = 0;
    UaVariant(output.positionOutput).toDouble(returnVal);
    return fmod(returnVal, 4000.0);
}

UaString ProductionModule::getSkillName(int& skillPos)
{
    if ((skillPos >= 0) && (skillPos < m_skillCount))
    {
        if (output.skillOutput[skillPos].dummy == false)
        {
            return output.skillOutput[skillPos].name;
        }
        else
        {
            UaString tmp;
            tmp = UaString("No skill active at position");
            tmp += UaString::number(skillPos);
            tmp += ".";
            return tmp;
        }
    }
    else
    {
        UaString tmp;
        tmp = UaString("No skill available at position");
        tmp += UaString::number(skillPos);
        tmp += ".";
        return tmp;
    }
}

UaString ProductionModule::getModuleName()
{
    return output.name;
}


bool ProductionModule::checkSkillReadyState(int& skillId)
{
    bool tmpBool = false;

    if ((skillId >= 0))
    {
        for (int i = 0; i < m_skillCount; i++)
        {
            if (output.skillOutput[i].id == skillId)
            {
                tmpBool = output.skillOutput[i].ready;
            }
        }
    }

    return tmpBool;
}

int ProductionModule::checkSkillState(int& skillId)
{
    int returnVal = -1;

    if (m_disconnected)
    {
        return SKILLMODULEERROR;
    }

    for (int i = 0; i < m_skillCount; i++)
    {
        if (output.skillOutput[i].id == skillId)
        {
            // Read state.
            int readCounter = 0;
            UaStatus status;
            UaDataValues returnValues;
            OpcUa_NodeId tmpNodeId;
            UaReadValueIds nodesToRead;

            nodesToRead.create(1);
            UaString baseNodeId = m_baseNodeId;
            baseNodeId += ".Output.SkillOutput.SkillOutput";
            baseNodeId += UaString::number(i);
            baseNodeId += ".";

            UaString nodeIdToRead = baseNodeId;
            nodeIdToRead += "Ready";
            UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[readCounter].NodeId);
            nodesToRead[readCounter].AttributeId = OpcUa_Attributes_Value;
            readCounter++;

            returnValues = m_pOpcuaGateway->read(nodesToRead);

            readCounter = 0;

            if (returnValues.length() == 1)
            {
                UaVariant(returnValues[0].Value).toBool(output.skillOutput[i].ready);
            }

            //
            if (output.skillOutput[i].ready)
            {
                returnVal = SKILLMODULEREADY;
                break;
            }
            else if (output.skillOutput[i].busy)
            {
                returnVal =  SKILLMODULEBUSY;
            }
            else if (output.skillOutput[i].done)
            {
                returnVal = SKILLMODULEDONE;
            }
            else if (output.skillOutput[i].error)
            {
                returnVal = SKILLMODULEERROR;
            }

            // TODO: error handling, if more than one of those markers is true.
        }
    }

    return returnVal;
}

void ProductionModule::buildSkillList()
{
    m_moduleSkillList.clear();

    for (int i = 0; i < m_skillCount; i++)
    {
        if (output.skillOutput[i].dummy == false)
        {
            m_moduleSkillList[output.skillOutput[i].id] = i;
        }
    }
}

void ProductionModule::skillStateChanged(int skillPos, int state)
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "skillStateChanged", Qt::QueuedConnection, Q_ARG(int, skillPos),
                                  Q_ARG(int, state));
        return;
    }

    if (m_skillRegistrationList.count(skillPos) > 0) //task registered for this skillpos
    {
        ISkillRegistration* pTask = m_skillRegistrationList[skillPos];

        if (output.skillOutput[skillPos].busy)
        {
            pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEBUSY);
        }

        else if (output.skillOutput[skillPos].done)
        {
            pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEDONE);
        }

        else if (output.skillOutput[skillPos].error)
        {
            pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEERROR);
        }

        else if (output.skillOutput[skillPos].ready)
        {
            pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEREADY);
        }

        //switch (state)
        //{
        //case SKILLMODULEBUSY:
        //    pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEBUSY);
        //    break;

        //case SKILLMODULEDONE:
        //    pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEDONE);
        //    break;

        //case SKILLMODULEERROR:
        //    pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEERROR);
        //    break;

        //case SKILLMODULEREADY:
        //    pTask->skillStateChanged(m_moduleNumber, skillPos, SKILLMODULEREADY);
        //    break;

        //default:
        //    break;
        //}
    }
    else // no task registered for this skill position
    {
    }
}

int ProductionModule::registerTaskForSkill(ISkillRegistration* pTask, int skillPos)
{
    int returnVal = -1;

    if (m_skillRegistrationList.count(skillPos) > 0)
    {
        QLOG_WARN() << "Skill at position " << skillPos << " is already registered with task id " <<
                    m_skillRegistrationList[skillPos]->getTaskId() ;
    }
    else
    {
        m_skillRegistrationList[skillPos] = pTask;
        createPoller(skillPos);
        returnVal = 0;
    }

    return returnVal;
}

void ProductionModule::createPoller(int skillPos)
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "createPoller", Qt::QueuedConnection, Q_ARG(int, skillPos));
        return;
    }

    m_skillStatePollerList[skillPos] = new SkillStatePoller(this, skillPos, m_pOpcuaGateway);
}

QMutex* ProductionModule::getMutex()
{
    return &m_mutex;
}


/*
* Following: OPC UA methods.
*/
void ProductionModule::writeConnectionTestInput(bool input)
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(1);
    UaVariant tmpValue;

    UaString baseNodeIdToWrite = m_baseNodeId;
    baseNodeIdToWrite += ".Input.ConnectionTestInput";

    tmpValue.setBool(OpcUa_Boolean(input));
    UaNodeId::fromXmlString(baseNodeIdToWrite).copyTo(&nodesToWrite[0].NodeId);
    nodesToWrite[0].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[0].Value.Value);

    m_pOpcuaGateway->write(nodesToWrite, 100);
}

int ProductionModule::checkConnectionTestOutput()
{
    // Read state.
    UaStatus status;
    UaDataValues returnValues;
    OpcUa_NodeId tmpNodeId;
    UaReadValueIds nodesToRead;

    nodesToRead.create(1);
    UaString baseNodeId = m_baseNodeId;
    baseNodeId += ".Output.ConnectionTestOutput";

    UaNodeId::fromXmlString(baseNodeId).copyTo(&nodesToRead[0].NodeId);
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;

    returnValues = m_pOpcuaGateway->read(nodesToRead, 500);

    OpcUa_Int32 returnVal = -1;

    if (returnValues.length() != 0)
    {
        UaVariant(returnValues[0].Value).toInt32(returnVal);
    }

    return returnVal;
}

void ProductionModule::serverReconnected()
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "serverReconnected", Qt::QueuedConnection);
        return;
    }

    setupOpcua();
}

int ProductionModule::getSkillState(int skillPos)
{
    int returnVal = -1;

    if (skillPos < 0 || skillPos > 10)
    {
        return returnVal;
    }
    else
    {
        if (output.error)
        {
            returnVal = SKILLMODULEERROR;
        }
        else if (output.skillOutput[skillPos].busy)
        {
            returnVal = SKILLMODULEBUSY;
        }
        else if (output.skillOutput[skillPos].ready)
        {
            returnVal = SKILLMODULEREADY;
        }
    }

    return returnVal;
}

void ProductionModule::createMonitoredItems()
{
    int clientHandleNumber;
    UaStatus status;
    // Prepare
    UaString baseNodeId = m_baseNodeId;
    baseNodeId += ".";

    // Following: input subscription
    // Create subscriptions
    // General
    //nodeIdToSubscribe = baseNodeId;
    //nodeIdToSubscribe += "Input.ConnectionTestInput";
    //createNodeStructure();
    //status = m_pOpcuaGateway->createSingleMonitoredItem(1000, m_moduleNumber,
    //         nodeToSubscribe);

    //nodeIdToSubscribe = baseNodeId;
    //nodeIdToSubscribe += "Input.EmergencyStop";
    //createNodeStructure();
    //status = m_pOpcuaGateway->createSingleMonitoredItem(1100, m_moduleNumber,
    //         nodeToSubscribe);

    //nodeIdToSubscribe = baseNodeId;
    //nodeIdToSubscribe += "Input.Mode";
    //createNodeStructure();
    //status = m_pOpcuaGateway->createSingleMonitoredItem(1200, m_moduleNumber,
    //         nodeToSubscribe);

    //nodeIdToSubscribe = baseNodeId;
    //nodeIdToSubscribe += "Input.PositionInput";
    //createNodeStructure();
    //status = m_pOpcuaGateway->createSingleMonitoredItem(1300, m_moduleNumber,
    //         nodeToSubscribe);

    //// Skills
    //for (int i = 0; i < SKILLCOUNT; i++)
    //{
    //    UaString tempNodeid = baseNodeId;
    //    tempNodeid += "Input.SkillInput.SkillInput";
    //    tempNodeid += UaString::number(i);
    //    tempNodeid += ".";

    //    tempNodeid += "Execute";
    //    nodeIdToSubscribe = tempNodeid;
    //    createNodeStructure();
    //    clientHandleNumber = (10000 + 4000 + i * 10 + 1);
    //    status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
    //             nodeToSubscribe);

    //    // Parameters
    //    for (int j = 0; j <= PARAMETERCOUNT; j++)
    //    {
    //        tempNodeid = baseNodeId;
    //        tempNodeid += "Input.SkillInput.SkillInput";
    //        tempNodeid += UaString::number(i);
    //        tempNodeid += ".";
    //        tempNodeid += "ParameterInput.Parameterinput";
    //        tempNodeid += UaString::number(j);
    //        tempNodeid += ".Value";
    //        nodeIdToSubscribe = tempNodeid;
    //        createNodeStructure();
    //        clientHandleNumber = (100000 + 40000 + i * 100 + j * 10);
    //        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
    //                 nodeToSubscribe);

    //        tempNodeid = baseNodeId;
    //        tempNodeid += "Input.SkillInput.SkillInput";
    //        tempNodeid += UaString::number(i);
    //        tempNodeid += ".";
    //        tempNodeid += "ParameterInput.Parameterinput";
    //        tempNodeid += UaString::number(j);
    //        tempNodeid += ".StringValue";
    //        nodeIdToSubscribe = tempNodeid;
    //        createNodeStructure();
    //        clientHandleNumber = (100000 + 40000 + i * 100 + j * 10 + 1);
    //        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
    //                 nodeToSubscribe);
    //    }
    //}

    // OUTPUT
    // General
    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.Connected";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(200, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.ConnectionTestOutput";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(201, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.CurrentTaskDescription";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(202, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.Dummy";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(203, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.Error";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(204, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.ErrorDescription";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(205, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.ErrorID";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(206, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.ID";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(207, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.IP";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(208, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.Idle";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(209, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.Name";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(210, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.PositionOutput";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(212, m_moduleNumber,
             nodeToSubscribe);

    nodeIdToSubscribe = baseNodeId;
    nodeIdToSubscribe += "Output.PositionSensor";
    createNodeStructure();
    status = m_pOpcuaGateway->createSingleMonitoredItem(211, m_moduleNumber,
             nodeToSubscribe);

    // TODO: Statevalues

    // Skills
    for (int i = 0; i < m_skillCount; i++)
    {
        UaString tempNodeid = baseNodeId;
        tempNodeid += "Output.SkillOutput.SkillOutput";
        tempNodeid += UaString::number(i);
        tempNodeid += ".";

        nodeIdToSubscribe = tempNodeid;
        nodeIdToSubscribe += "Activated";
        createNodeStructure();
        clientHandleNumber = (2200 + i * 10 + 1);
        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                 nodeToSubscribe);

        nodeIdToSubscribe = tempNodeid;
        nodeIdToSubscribe += "Busy";
        createNodeStructure();
        clientHandleNumber = (2200 + i * 10 + 2);
        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                 nodeToSubscribe);

        nodeIdToSubscribe = tempNodeid;
        nodeIdToSubscribe += "Done";
        createNodeStructure();
        clientHandleNumber = (2200 + i * 10 + 3);
        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                 nodeToSubscribe);

        nodeIdToSubscribe = tempNodeid;
        nodeIdToSubscribe += "Dummy";
        createNodeStructure();
        clientHandleNumber = (2200 + i * 10 + 4);
        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                 nodeToSubscribe);

        nodeIdToSubscribe = tempNodeid;
        nodeIdToSubscribe += "Error";
        createNodeStructure();
        clientHandleNumber = (2200 + i * 10 + 5);
        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                 nodeToSubscribe);

        nodeIdToSubscribe = tempNodeid;
        nodeIdToSubscribe += "ID";
        createNodeStructure();
        clientHandleNumber = (2200 + i * 10 + 6);
        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                 nodeToSubscribe);

        nodeIdToSubscribe = tempNodeid;
        nodeIdToSubscribe += "Name";
        createNodeStructure();
        clientHandleNumber = (2200 + i * 10 + 7);
        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                 nodeToSubscribe);

        nodeIdToSubscribe = tempNodeid;
        nodeIdToSubscribe += "Ready";
        createNodeStructure();
        clientHandleNumber = (2200 + i * 10 + 9);
        status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                 nodeToSubscribe);

        UaString baseNodeIdParam = baseNodeId;
        baseNodeIdParam += "Output.SkillOutput.SkillOutput";
        baseNodeIdParam += UaString::number(i);
        baseNodeIdParam += ".";
        baseNodeIdParam += "ParameterOutput.ParameterOutput";

        // Parameters
        for (int j = 0; j <= PARAMETERCOUNTOSUBSCRIBE; j++)
        {
            tempNodeid = baseNodeIdParam;
            tempNodeid += UaString::number(j);
            tempNodeid += ".Default";
            nodeIdToSubscribe = tempNodeid;
            createNodeStructure();
            clientHandleNumber = (20000 + 2000 + i * 100 + j * 10 + 1);
            status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                     nodeToSubscribe);

            tempNodeid = baseNodeIdParam;
            tempNodeid += UaString::number(j);
            tempNodeid += ".Dummy";
            nodeIdToSubscribe = tempNodeid;
            createNodeStructure();
            clientHandleNumber = (20000 + 2000 + i * 100 + j * 10 + 2);
            status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                     nodeToSubscribe);

            tempNodeid = baseNodeIdParam;
            tempNodeid += UaString::number(j);
            tempNodeid += ".ID";
            nodeIdToSubscribe = tempNodeid;
            createNodeStructure();
            clientHandleNumber = (20000 + 2000 + i * 100 + j * 10 + 3);
            status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                     nodeToSubscribe);

            tempNodeid = baseNodeIdParam;
            tempNodeid += UaString::number(j);
            tempNodeid += ".MaxValue";
            nodeIdToSubscribe = tempNodeid;
            createNodeStructure();
            clientHandleNumber = (20000 + 2000 + i * 100 + j * 10 + 4);
            status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                     nodeToSubscribe);

            tempNodeid = baseNodeIdParam;
            tempNodeid += UaString::number(j);
            tempNodeid += ".MinValue";
            nodeIdToSubscribe = tempNodeid;
            createNodeStructure();
            clientHandleNumber = (20000 + 2000 + i * 100 + j * 10 + 5);
            status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                     nodeToSubscribe);

            tempNodeid = baseNodeIdParam;
            tempNodeid += UaString::number(j);
            tempNodeid += ".Name";
            nodeIdToSubscribe = tempNodeid;
            createNodeStructure();
            clientHandleNumber = (20000 + 2000 + i * 100 + j * 10 + 6);
            status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                     nodeToSubscribe);

            tempNodeid = baseNodeIdParam;
            tempNodeid += UaString::number(j);
            tempNodeid += ".Required";
            nodeIdToSubscribe = tempNodeid;
            createNodeStructure();
            clientHandleNumber = (20000 + 2000 + i * 100 + j * 10 + 7);
            status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                     nodeToSubscribe);

            tempNodeid = baseNodeIdParam;
            tempNodeid += UaString::number(j);
            tempNodeid += ".Unit";
            nodeIdToSubscribe = tempNodeid;
            createNodeStructure();
            clientHandleNumber = (20000 + 2000 + i * 100 + j * 10 + 8);
            status = m_pOpcuaGateway->createSingleMonitoredItem(clientHandleNumber, m_moduleNumber,
                     nodeToSubscribe);
        }
    }

    QLOG_DEBUG() "Finished creating the monitored items for module number " << m_moduleNumber << " (" <<
            getModuleName().toUtf8() << ").";
}

void ProductionModule::writeModuleInput()
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(5 + m_skillCount * (1    + PARAMETERCOUNT * 1)); // Fill in number
    int writeCounter = 0;
    UaVariant tmpValue;

    UaString baseNodeIdToWrite = "ns=4;s=MI5.Module";
    baseNodeIdToWrite += UaString::number(m_moduleNumber);
    baseNodeIdToWrite += ".Input.";

    UaString tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "ConnectionTestInput";
    tmpValue.setBool(input.connectionTestInput);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "EmergencyStop";
    tmpValue.setBool(input.emergencyStop);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();


    tmpNodeId = baseNodeIdToWrite; // TODO!
    tmpNodeId += "Mode";
    tmpValue.setInt16(input.moduleMode);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();


    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "PositionInput";
    tmpValue.setDouble(input.positionInput);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();


    for (int i = 0; i < m_skillCount; i++)
    {
        UaString baseSkillNodeId = baseNodeIdToWrite;
        baseSkillNodeId += "SkillInput.SkillInput";
        baseSkillNodeId += UaString::number(i);
        baseSkillNodeId += ".";

        tmpNodeId = baseSkillNodeId;
        tmpNodeId += "Execute";
        tmpValue.setBool(input.skillInput[i].execute);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();


        for (int j = 0; j < PARAMETERCOUNT; j++)
        {
            UaString baseParamNodeId = baseSkillNodeId;
            baseParamNodeId += "ParameterInput.ParameterInput";
            baseParamNodeId += UaString::number(j);
            baseParamNodeId += ".";

            tmpNodeId = baseParamNodeId;
            tmpNodeId += "Value";
            tmpValue.setDouble(input.skillInput[i].parameterInput[j].value);
            UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
            nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
            OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
            writeCounter++;
            tmpValue.clear();

            //tmpNodeId = baseParamNodeId; // TODO!
            //tmpNodeId += "StringValue";
            //tmpValue.setString(input.skillInput[i].parameterInput[j].string);
            //UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
            //nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
            //OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
            //writeCounter++;
            //tmpValue.clear();
        }
    }

    // Write!
    m_pOpcuaGateway->write(nodesToWrite);
}

void ProductionModule::writeSkillInput(int skillPos)
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(1 + PARAMETERCOUNT * 2);
    int writeCounter = 0;
    UaVariant tmpValue;

    UaString baseNodeIdToWrite = m_baseNodeId;
    baseNodeIdToWrite += ".Input.";

    UaString baseSkillNodeId = baseNodeIdToWrite;
    baseSkillNodeId += "SkillInput.SkillInput";
    baseSkillNodeId += UaString::number(skillPos);
    baseSkillNodeId += ".";

    UaString tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Execute";
    tmpValue.setBool(input.skillInput[skillPos].execute);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();


    for (int j = 0; j < PARAMETERCOUNT; j++)
    {
        UaString baseParamNodeId = baseSkillNodeId;
        baseParamNodeId += "ParameterInput.ParameterInput";
        baseParamNodeId += UaString::number(j);
        baseParamNodeId += ".";

        tmpNodeId = baseParamNodeId;
        tmpNodeId += "Value";
        tmpValue.setDouble(input.skillInput[skillPos].parameterInput[j].value);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();

        tmpNodeId = baseParamNodeId;
        tmpNodeId += "StringValue";
        tmpValue.setString(input.skillInput[skillPos].parameterInput[j].string);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();
    }


    // Write!
    m_pOpcuaGateway->write(nodesToWrite);
}




void ProductionModule::createNodeStructure()
{
    nodeToSubscribe.clear();
    nodeToSubscribe.resize(1);
    UaNodeId::fromXmlString(nodeIdToSubscribe).copyTo(&nodeToSubscribe[0]);
}


void ProductionModule::subscriptionDataChange(OpcUa_UInt32 clientSubscriptionHandle,
        const UaDataNotifications& dataNotifications,
        const UaDiagnosticInfos&     diagnosticInfos)
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


// Dont look.
void ProductionModule::moduleDataChange(const UaDataNotifications& dataNotifications)
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
            //if (dataNotifications[i].ClientHandle == 1000)
            //{
            //    tempValue.toBool(input.connectionTestInput);
            //}

            //else if (dataNotifications[i].ClientHandle == 1100)
            //{
            //    tempValue.toBool(input.emergencyStop);
            //}

            //else if (dataNotifications[i].ClientHandle == 1200)
            //{
            //    //tempValue.touabyt(input.moduleMode); TODO
            //}

            //else if (dataNotifications[i].ClientHandle == 1300)
            //{
            //    tempValue.toDouble(input.positionInput);
            //}

            // Output
            if (dataNotifications[i].ClientHandle == 200)
            {
                tempValue.toBool(output.connected);
            }

            else if (dataNotifications[i].ClientHandle == 201)
            {
                tempValue.toBool(output.connectionTestOutput);
            }

            else if (dataNotifications[i].ClientHandle == 202)
            {
                output.currentTaskDescription = UaString::UaString(tempValue.toString());
            }

            else if (dataNotifications[i].ClientHandle == 203)
            {
                tempValue.toBool(output.dummy);
            }

            else if (dataNotifications[i].ClientHandle == 204)
            {
                tempValue.toBool(output.error);

                if (output.error)
                {
                    emit errorOccured();
                }
            }

            else if (dataNotifications[i].ClientHandle == 205)
            {
                output.errorDescription = UaString::UaString(tempValue.toString());
            }

            else if (dataNotifications[i].ClientHandle == 206)
            {
                tempValue.toUInt32(output.errorId);
            }

            else if (dataNotifications[i].ClientHandle == 207)
            {
                tempValue.toUInt32(output.id);
            }

            else if (dataNotifications[i].ClientHandle == 208)
            {
                output.ip = UaString::UaString(tempValue.toString());
            }

            else if (dataNotifications[i].ClientHandle == 209)
            {
                tempValue.toBool(output.idle);
            }

            else if (dataNotifications[i].ClientHandle == 210)
            {
                output.name = UaString::UaString(tempValue.toString());
            }

            else if (dataNotifications[i].ClientHandle == 211)
            {
                tempValue.toBool(output.positionSensor);
            }

            else if (dataNotifications[i].ClientHandle == 212)
            {
                tempValue.toDouble(output.positionOutput);
            }

            else
            {
                for (int k = 0; k < m_skillCount; k++) // Get skills
                {
                    // Input
                    if ((dataNotifications[i].ClientHandle % (14001 + k * 10)) == 0)
                    {
                        tempValue.toBool(input.skillInput[k].execute);
                    }

                    // Output
                    if ((dataNotifications[i].ClientHandle % (2200 + k * 10 + 1)) == 0)
                    {
                        tempValue.toBool(output.skillOutput[k].activated);
                    }

                    else if ((dataNotifications[i].ClientHandle % (2200 + k * 10 + 2)) == 0)
                    {
                        tempValue.toBool(output.skillOutput[k].busy);
                        //skillStateChanged(k, SKILLMODULEBUSY);
                    }

                    else if ((dataNotifications[i].ClientHandle % (2200 + k * 10 + 3)) == 0)
                    {
                        tempValue.toBool(output.skillOutput[k].done);
                        //skillStateChanged(k, SKILLMODULEDONE);
                    }

                    else if ((dataNotifications[i].ClientHandle % (2200 + k * 10 + 4)) == 0)
                    {
                        tempValue.toBool(output.skillOutput[k].dummy);
                    }

                    else if ((dataNotifications[i].ClientHandle % (2200 + k * 10 + 5)) == 0)
                    {
                        tempValue.toBool(output.skillOutput[k].error);
                        //skillStateChanged(k, SKILLMODULEERROR);
                    }

                    else if ((dataNotifications[i].ClientHandle % (2200 + k * 10 + 6)) == 0)
                    {
                        tempValue.toUInt32(output.skillOutput[k].id);
                    }

                    else if ((dataNotifications[i].ClientHandle % (2200 + k * 10 + 7)) == 0)
                    {
                        output.skillOutput[k].name = UaString::UaString(tempValue.toString());
                    }

                    else if ((dataNotifications[i].ClientHandle % (2200 + k * 10 + 9)) == 0)
                    {
                        tempValue.toBool(output.skillOutput[k].ready);
                        //skillStateChanged(k, SKILLMODULEREADY);
                    }

                    else
                    {
                        for (int l = 0; l < PARAMETERCOUNT; l++) // Get parameters
                        {
                            //// Input
                            //if ((dataNotifications[i].ClientHandle % (100000 + 40000 + k * 100 + l * 10)) == 0)
                            //{
                            //    tempValue.toDouble(input.skillInput[k].parameterInput[l].value);
                            //}

                            //else if ((dataNotifications[i].ClientHandle % (100000 + 40000 + k * 100 + l * 10 + 1)) == 0)
                            //{
                            //    input.skillInput[k].parameterInput[l].string = UaString::UaString(tempValue.toString());
                            //}
                            // Output
                            if ((dataNotifications[i].ClientHandle % (20000 + 2000 + k * 100 + l * 10 + 1)) == 0)
                            {
                                tempValue.toDouble(output.skillOutput[k].parameterOutput[l].defaultParameter);
                            }
                            else if ((dataNotifications[i].ClientHandle % (20000 + 2000 + k * 100 + l * 10 +
                                      2)) == 0)
                            {
                                tempValue.toBool(output.skillOutput[k].parameterOutput[l].dummy);
                            }
                            else if ((dataNotifications[i].ClientHandle % (20000 + 2000 + k * 100 + l * 10 +
                                      3)) == 0)
                            {
                                tempValue.toUInt32(output.skillOutput[k].parameterOutput[l].id);
                            }
                            else if ((dataNotifications[i].ClientHandle % (20000 + 2000 + k * 100 + l * 10 + 4)) == 0)
                            {
                                tempValue.toDouble(output.skillOutput[k].parameterOutput[l].maxValue);
                            }
                            else if ((dataNotifications[i].ClientHandle % (20000 + 2000 + k * 100 + l * 10 + 5)) == 0)
                            {
                                tempValue.toDouble(output.skillOutput[k].parameterOutput[l].minValue);
                            }
                            else if ((dataNotifications[i].ClientHandle % (20000 + 2000 + k * 100 + l * 10 + 6)) == 0)
                            {
                                output.skillOutput[k].parameterOutput[l].name = UaString::UaString(tempValue.toString());
                            }
                            else if ((dataNotifications[i].ClientHandle % (20000 + 2000 + k * 100 + l * 10 +
                                      7)) == 0)
                            {
                                tempValue.toBool(output.skillOutput[k].parameterOutput[l].required);
                            }
                            else if ((dataNotifications[i].ClientHandle % (20000 + 2000 + k * 100 + l * 10 + 8)) == 0)
                            {
                                output.skillOutput[k].parameterOutput[l].unit = UaString::UaString(tempValue.toString());
                            }
                        }
                    }
                }
            }
        }
    }
}

bool ProductionModule::isReserved()
{
    return false;
}

bool ProductionModule::isBlocked()
{
    return false;
}

void ProductionModule::checkMoverState(int skillPos)
{

}

void ProductionModule::changeModuleMode(int mode)
{
    input.moduleMode = mode;

    UaWriteValues nodesToWrite;
    nodesToWrite.create(1); // Fill in number
    int writeCounter = 0;
    UaVariant tmpValue;

    UaString baseNodeIdToWrite = m_baseNodeId;
    baseNodeIdToWrite += ".Input.";
    baseNodeIdToWrite += "Mode";
    tmpValue.setInt16(input.moduleMode);
    UaNodeId::fromXmlString(baseNodeIdToWrite).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    // Write!
    m_pOpcuaGateway->write(nodesToWrite);
}

UaString ProductionModule::getBaseNodeId()
{
    return m_baseNodeId;
}

int ProductionModule::getErrorId()
{
    OpcUa_Int16 returnVal = -1;

    // Read state.
    UaStatus status;
    UaDataValues returnValues;
    OpcUa_NodeId tmpNodeId;
    UaReadValueIds nodesToRead;

    nodesToRead.create(1);
    UaString nodeIdToRead = m_baseNodeId;
    nodeIdToRead += ".Output.ErrorID";

    UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[0].NodeId);
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;

    returnValues = m_pOpcuaGateway->read(nodesToRead);

    if (returnValues.length() == 1)
    {
        UaVariant(returnValues[0].Value).toInt16(returnVal);
    }

    return (int)returnVal;
}

