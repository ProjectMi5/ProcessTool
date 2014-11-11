#include <Mi5_ProcessTool/include/MessageFeeder.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <qdatetime.h>

MessageFeeder::MessageFeeder(OpcuaGateway* pGateway, int moduleNumber) : m_feedId(1),
    m_feedCounter(0), m_pGateway(pGateway), m_moduleNumber(moduleNumber)
{
    resetList();
}

MessageFeeder::~MessageFeeder()
{
}

void MessageFeeder::write(UaString string, messageFeedLevel level)
{
    UaString timestamp = UaString(
                             QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toUtf8().constData());
    writeToOpcua(string, level, timestamp);
}

void MessageFeeder::writeToOpcua(UaString string, messageFeedLevel level, UaString timestamp)
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(4);
    int writeCounter = 0;
    UaVariant tmpValue;

    UaString baseNodeIdToWrite = "ns=4;s=MI5.MessageFeed[";
    baseNodeIdToWrite += UaString::number(m_feedCounter);
    baseNodeIdToWrite += "].";

    UaString tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "ID";
    tmpValue.setInt32(m_feedId);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "Level";
    tmpValue.setInt32(level);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "Message";
    tmpValue.setString(string);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseNodeIdToWrite;
    tmpNodeId += "Timestamp";
    tmpValue.setString(timestamp);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    // Write!
    m_pGateway->write(nodesToWrite);

    m_feedId++;
    m_feedCounter++;
    m_feedCounter = m_feedCounter % LISTSIZE;
}

void MessageFeeder::resetList()
{
    UaString string = "";

    for (int i = 0; i < LISTSIZE; i++)
    {
        m_feedId = 0;
        m_feedCounter = 0;
        writeToOpcua(string, msgClear, string);
    }

    m_feedId = 1;
    m_feedCounter = 0;
}
