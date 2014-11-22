#include <Mi5_ProcessTool/include/ProductionModules/SkillStatePollerManual.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>

SkillStatePollerManual::SkillStatePollerManual(IProductionModule* productionModule,
        int moduleNumber,
        int skillPos,
        OpcuaGateway* pGateway) : SkillStatePoller(productionModule, skillPos, pGateway),
    m_moduleNumber(moduleNumber)
{

}

SkillStatePollerManual::~SkillStatePollerManual()
{

}

void SkillStatePollerManual::checkSkillState()
{
    // Read states.
    UaDataValues returnValues;
    UaReadValueIds nodesToRead;
    int readCounter = 0;

    nodesToRead.create(3);
    UaString baseNodeId = "MI5.Module";
    baseNodeId += UaString::number(m_moduleNumber);
    baseNodeId += "Manual.";

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
