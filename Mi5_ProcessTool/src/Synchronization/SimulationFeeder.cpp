#include <Mi5_ProcessTool/include/Synchronization/SimulationFeeder.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>


SimulationFeeder::SimulationFeeder(OpcuaGateway* pOpcuaGateway, int moduleNumber,
                                   std::map<int, IProductionModule*> moduleList,
                                   MessageFeeder* pMsgFeeder) : m_pGateway(pOpcuaGateway), m_moduleList(moduleList),
    m_pMsgFeeder(pMsgFeeder)
{
    for (int i = 0; i < 11; i++)
    {
        data.XPosition[i] = 0;
    }

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(cyclicAction()));
    m_timer->start(1000);

    moveToThread(&m_thread);
    m_thread.setObjectName("ThreadSimulationFeeder");
    m_thread.start();
}

SimulationFeeder::~SimulationFeeder()
{

}

void SimulationFeeder::cyclicAction()
{
    updateModuleData();
    writePositionInfo();
}

void SimulationFeeder::writePositionInfo()
{
    UaWriteValues nodesToWrite;
    nodesToWrite.create(11 + 4 * 3);
    int writeCounter = 0;
    UaVariant tmpValue;


    UaString baseSkillNodeId = "ns=4;s=MI5.Simulation.";

    for (int i = 0; i < 11; i++)
    {
        UaString tmpNodeId = baseSkillNodeId;
        tmpNodeId += "XPosition";
        tmpNodeId += UaString::number(i);
        tmpValue.setDouble(data.XPosition[i]);
        UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
        nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
        OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
        writeCounter++;
        tmpValue.clear();
    }

    UaString tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Cocktail.Busy";
    tmpValue.setBool(data.cocktail.busy);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Cocktail.Error";
    tmpValue.setBool(data.cocktail.error);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Cocktail.Ready";
    tmpValue.setBool(data.cocktail.ready);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Cookie.Busy";
    tmpValue.setBool(data.cookie.busy);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Cookie.Error";
    tmpValue.setBool(data.cookie.error);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Cookie.Ready";
    tmpValue.setBool(data.cookie.ready);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Topping_Beckhoff.Busy";
    tmpValue.setBool(data.toppingBeckhoff.busy);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Topping_Beckhoff.Error";
    tmpValue.setBool(data.toppingBeckhoff.error);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Topping_Beckhoff.Ready";
    tmpValue.setBool(data.toppingBeckhoff.ready);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Topping_Bosch.Busy";
    tmpValue.setBool(data.toppingBosch.busy);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Topping_Bosch.Error";
    tmpValue.setBool(data.toppingBosch.error);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();
    tmpNodeId = baseSkillNodeId;
    tmpNodeId += "Topping_Bosch.Ready";
    tmpValue.setBool(data.toppingBosch.ready);
    UaNodeId::fromXmlString(tmpNodeId).copyTo(&nodesToWrite[writeCounter].NodeId);
    nodesToWrite[writeCounter].AttributeId = OpcUa_Attributes_Value;
    OpcUa_Variant_CopyTo(tmpValue, &nodesToWrite[writeCounter].Value.Value);
    writeCounter++;
    tmpValue.clear();

    // Write!
    m_pGateway->write(nodesToWrite);
}

void SimulationFeeder::updateModuleData()
{
    for (std::map<int, IProductionModule*>::iterator it = m_moduleList.begin();
         it != m_moduleList.end(); it++)
    {
        int cocktailState = -1;

        switch (it->first)
        {
        case MODULENUMBERCOOKIESEPARATOR:
            data.XPosition[0] = m_moduleList[MODULENUMBERCOOKIESEPARATOR]->getModulePosition();

            switch (m_moduleList[MODULENUMBERCOOKIESEPARATOR]->getSkillState(0))
            {
            case SKILLMODULEERROR:
                data.cookie.error = true;
                data.cookie.busy = false;
                data.cookie.ready = false;
                break;

            case SKILLMODULEBUSY:
                data.cookie.busy = true;
                data.cookie.ready = false;
                data.cookie.error = false;
                break;

            case SKILLMODULEREADY:
                data.cookie.ready = true;
                data.cookie.busy = false;
                data.cookie.error = false;
                break;

            default:
                break;
            }

            break;

        case MODULENUMBERCREMEBECKHOFF:
            data.XPosition[1] = m_moduleList[MODULENUMBERCREMEBECKHOFF]->getModulePosition();

            switch (m_moduleList[MODULENUMBERCREMEBECKHOFF]->getSkillState(0))
            {
            case SKILLMODULEERROR:
                data.toppingBeckhoff.error = true;
                data.toppingBeckhoff.busy = false;
                data.toppingBeckhoff.ready = false;
                break;

            case SKILLMODULEBUSY:
                data.toppingBeckhoff.busy = true;
                data.toppingBeckhoff.ready = false;
                data.toppingBeckhoff.error = false;
                break;

            case SKILLMODULEREADY:
                data.toppingBeckhoff.ready = true;
                data.toppingBeckhoff.busy = false;
                data.toppingBeckhoff.error = false;
                break;

            default:
                break;
            }

            break;

        case MODULENUMBERCREMEBOSCH:
            data.XPosition[2] = m_moduleList[MODULENUMBERCREMEBOSCH]->getModulePosition();

            switch (m_moduleList[MODULENUMBERCREMEBOSCH]->getSkillState(0))
            {
            case SKILLMODULEERROR:
                data.toppingBosch.error = true;
                data.toppingBosch.busy = false;
                data.toppingBosch.ready = false;
                break;

            case SKILLMODULEBUSY:
                data.toppingBosch.busy = true;
                data.toppingBosch.ready = false;
                data.toppingBosch.error = false;
                break;

            case SKILLMODULEREADY:
                data.toppingBosch.ready = true;
                data.toppingBosch.busy = false;
                data.toppingBosch.error = false;
                break;

            default:
                break;
            }

            break;

        case MODULENUMBERCOCKTAIL:
            data.XPosition[3] = m_moduleList[MODULENUMBERCOCKTAIL]->getModulePosition();

            cocktailState = -1;

            for (int i = 0; i < 8; i++)
            {
                cocktailState = m_moduleList[MODULENUMBERCOCKTAIL]->getSkillState(i);

                if ((cocktailState == SKILLMODULEERROR) || (cocktailState == SKILLMODULEBUSY))
                {
                    break;
                }
            }

            switch (cocktailState)
            {
            case SKILLMODULEERROR:
                data.cocktail.error = true;
                data.cocktail.busy = false;
                data.cocktail.ready = false;
                break;

            case SKILLMODULEBUSY:
                data.cocktail.busy = true;
                data.cocktail.ready = false;
                data.cocktail.error = false;
                break;

            case SKILLMODULEREADY:
                data.cocktail.ready = true;
                data.cocktail.busy = false;
                data.cocktail.error = false;
                break;

            default:
                break;
            }

            break;

        default:
            break;
        }
    }
}

