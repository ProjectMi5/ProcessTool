#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <Mi5_ProcessTool/include/SkillStatePoller.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>

SkillStatePoller::SkillStatePoller(IProductionModule* productionModule,
                                   int skillPos, OpcuaGateway* pGateway) : m_pModule(productionModule), m_skillPos(skillPos),
    m_pGateway(pGateway)
{
    //QLOG_DEBUG() << "Created SkillStatePoller for module " << m_pModule->getModuleName().toUtf8();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkSkillState()));
    m_timer->start(1000); //TODO: adjust values

    moveToThread(&m_thread);
    m_thread.start();
}

SkillStatePoller::~SkillStatePoller()
{
    //QLOG_DEBUG() << "Deleted SkillStatePoller for module " << m_pModule->getModuleName().toUtf8();
}

void SkillStatePoller::checkSkillState()
{
    // Read states.
    UaDataValues returnValues;
    UaReadValueIds nodesToRead;
    int readCounter = 0;

    nodesToRead.create(3);
    UaString baseNodeId = m_pModule->getBaseNodeId();
    baseNodeId += ".Output.SkillOutput.SkillOutput";
    baseNodeId += UaString::number(m_skillPos);
    baseNodeId += ".";

    UaString tmpNodeId = baseNodeId;
    tmpNodeId += "Busy";
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    tmpNodeId = baseNodeId;
    tmpNodeId += "Done";
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    tmpNodeId = baseNodeId;
    tmpNodeId += "Busy";
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToRead[readCounter].NodeId);
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;
    readCounter++;

    returnValues = m_pGateway->read(nodesToRead, 500); //TODO: adjust values.

    if (returnValues.length() == 0)
    {
        return;
    }

    readCounter = 0;
    UaVariant(returnValues[readCounter].Value).toBool(m_busy);
    readCounter++;
    UaVariant(returnValues[readCounter].Value).toBool(m_done);
    readCounter++;
    UaVariant(returnValues[readCounter].Value).toBool(m_error);
    readCounter++;

    evalState();
}

void SkillStatePoller::evalState()
{
    int state = -1;

    if (m_done)
    {
        state = SKILLMODULEDONE;
    }
    else if (m_busy)
    {
        state = SKILLMODULEBUSY;
    }
    else if (m_error)
    {
        state = SKILLMODULEERROR;
    }

    //QMetaObject::invokeMethod(m_pModule, "skillStateChanged", Qt::QueuedConnection, Q_ARG(int,
    //                          m_skillPos), Q_ARG(int, state));
    m_pModule->skillStateChanged(m_skillPos, state);
}

