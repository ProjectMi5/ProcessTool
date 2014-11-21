#include <Mi5_ProcessTool/include/Synchronization/SimulationFeeder.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>


SimulationFeeder::SimulationFeeder(OpcuaGateway* pOpcuaGateway, int moduleNumber,
                                   std::map<int, IProductionModule*> moduleList,
                                   MessageFeeder* pMsgFeeder) : m_pGateway(pOpcuaGateway), m_moduleList(moduleList),
    m_pMsgFeeder(pMsgFeeder)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(cyclicAction()));
    m_timer->start(4000);

    moveToThread(&m_thread);
    m_thread.setObjectName("ThreadSimulationFeeder");
    m_thread.start();
}

SimulationFeeder::~SimulationFeeder()
{

}

void SimulationFeeder::cyclicAction()
{
    getPositions();
    writePositionInfo();
}

void SimulationFeeder::writePositionInfo()
{
    //UaWriteValues nodesToWrite;
    //nodesToWrite.create(11);
    //int writeCounter = 0;
    //UaVariant tmpValue;


    //UaString baseSkillNodeId = "MI5.Simulation.";

    //for (int i = 0; i < 11; i++)
    //{
    //    UaString tmpNodeId = baseSkillNodeId;
    //    tmpNodeId += "XPosition";
    //    tmpNodeId += UaString::number(i);
    //    tmpValue.setDouble(data.XPosition[i]);
    //    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    //    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    //    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    //    writeCounter++;
    //    tmpValue.clear();
    //}

    //// Write!
    //m_pGateway->write(nodesToWrite);
}


void SimulationFeeder::getPositions()
{
    for (std::map<int, IProductionModule*>::iterator it = m_moduleList.begin();
         it != m_moduleList.end(); it++)
    {
        switch (it->first)
        {
        case MODULENUMBERCOOKIESEPARATOR:
            data.XPosition[0] = m_moduleList[MODULENUMBERCOOKIESEPARATOR]->getModulePosition();
            break;

        case MODULENUMBERCREMEBECKHOFF:
            data.XPosition[1] = m_moduleList[MODULENUMBERCREMEBECKHOFF]->getModulePosition();
            break;

        case MODULENUMBERCREMEBOSCH:
            data.XPosition[2] = m_moduleList[MODULENUMBERCREMEBOSCH]->getModulePosition();
            break;

        case MODULENUMBERCOCKTAIL:
            data.XPosition[3] = m_moduleList[MODULENUMBERCOCKTAIL]->getModulePosition();
            break;

        default:
            break;
        }
    }
}

