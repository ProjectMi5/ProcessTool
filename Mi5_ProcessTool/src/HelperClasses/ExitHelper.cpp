#include <Mi5_ProcessTool/include/HelperClasses/ExitHelper.h>

ExitHelper::ExitHelper()
{
    m_exitTimer = new QTimer(this);
    connect(m_exitTimer, SIGNAL(timeout()), this, SLOT(timerTriggered()));
    m_exitTimer->start(5000);
    m_pGateway = new OpcuaGateway(UaString("opc.tcp://192.168.42.51:4840"));
    m_pGateway->connect();

    moveToThread(&m_thread);
    m_thread.start();
}

ExitHelper::~ExitHelper()
{

}

void ExitHelper::timerTriggered()
{
    bool resetRequested = readExitDemand();

    if (resetRequested)
    {
        bool exitDemandReset = resetExitDemand();

        if (exitDemandReset)
        {
            quitApplication();
        }
    }

}

bool ExitHelper::readExitDemand()
{
    OpcUa_Boolean returnVal = false;
    // Read state.
    UaStatus status;
    UaDataValues returnValues;
    OpcUa_NodeId tmpNodeId;
    UaReadValueIds nodesToRead;

    nodesToRead.create(1);
    UaString nodeIdToRead = "ns=4;s=MI5.ResetProcessTool";

    UaNodeId::fromXmlString(nodeIdToRead).copyTo(&nodesToRead[0].NodeId);
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;

    returnValues = m_pGateway->read(nodesToRead);

    if (returnValues.length() == 1)
    {
        UaVariant(returnValues[0].Value).toBool(returnVal);
        OpcUa_Boolean asdf;
    }

    return (bool)returnVal;
}

bool ExitHelper::resetExitDemand()
{
    bool returnVal = false;

    UaWriteValues nodesToWrite;
    UaStatus status;
    nodesToWrite.create(1);
    UaVariant tmpValue;

    UaString baseNodeIdToWrite = "ns=4;s=MI5.ResetProcessTool";

    tmpValue.setBool(false);
    UaNodeId::fromXmlString(baseNodeIdToWrite).copyTo(&nodesToWrite[0].NodeId);
    nodesToWrite[0].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[0].Value.Value);

    status = m_pGateway->write(nodesToWrite);

    if (status.isGood())
    {
        returnVal = true;
    }

    return returnVal;
}

void ExitHelper::quitApplication()
{
    qApp->exit(1337);
}



