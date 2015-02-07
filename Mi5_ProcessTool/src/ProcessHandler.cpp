#include <Mi5_ProcessTool/include/ProcessHandler.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <QApplication>

static const UaString MAINSERVER("opc.tcp://192.168.192.116:4840"); //116 Production //132 dummy

ProcessHandler::ProcessHandler()
{
    m_xts = NULL;
    m_cremeModule = NULL;
    m_taskModule = NULL;

    m_xts_enabled = true;
    m_cookie_enabled = true;
    m_topping_beckhoff_enabled = true;
    m_topping_bosch_enabled = true;
    m_cocktail_enabled = true;
    m_virtualModules_enabled = true;
    m_init = true;
    m_simuEnabled = true;
    m_enableInOutput = true;

    m_moduleSkillList.clear();
    m_gatewayList.clear();
    m_gatewayList.clear();
    m_productionModuleList.clear();

    start();
}

ProcessHandler::~ProcessHandler()
{
    //TODO: delete gateways and modules

    if (m_xts)
    {
        delete m_xts;
        m_xts = NULL;
    }


    if (m_cremeModule)
    {
        delete m_cremeModule;
        m_cremeModule = NULL;
    }

    if (m_taskModule)
    {
        delete m_taskModule;
        m_taskModule = NULL;
    }


    // Cleanup the UA Stack platform layer
    UaPlatformLayer::cleanup();
}

void ProcessHandler::start()
{
    UaStatus status;
    QLOG_DEBUG() << "Starting MI5 Process Tool.." ;
    status = build();

    if (status.isGood())
    {
        run();
    }

    return;
}

UaStatus ProcessHandler::build()
{

    UaStatus status;

    // Initialize the UA Stack platform layer
    UaPlatformLayer::init();

    // Create instance of OpcuaGateway
    //1. Stelle ÄNDERN
    if (m_simuEnabled)
    {
        m_gatewayList[MODULENUMBERSIMULATIONFEEDER] = new OpcuaGateway(
            UaString("opc.tcp://192.168.192.117:4840"));
    }

    if (m_cookie_enabled)
    {
        m_gatewayList[MODULENUMBERCOOKIESEPARATOR] = new OpcuaGateway(
            UaString("opc.tcp://192.168.192.136:4840"));
    }

    if (m_enableInOutput)
    {
        m_gatewayList[INPUTMODULE] = new OpcuaGateway(UaString("opc.tcp://192.168.192.117:4840"));
        m_gatewayList[OUTPUTMODULE] = new OpcuaGateway(UaString("opc.tcp://192.168.192.117:4840"));

    }

    if (m_xts_enabled)
    {
        m_gatewayList[MODULENUMBERXTS1] = new OpcuaGateway(UaString("opc.tcp://192.168.192.137:4840"));
        m_gatewayList[MODULENUMBERXTS2] = new OpcuaGateway(UaString("opc.tcp://192.168.192.137:4840"));
        m_gatewayList[MODULENUMBERXTS3] = new OpcuaGateway(UaString("opc.tcp://192.168.192.137:4840"));
    }

    if (m_virtualModules_enabled)
    {
        m_gatewayList[MODULEX] = new OpcuaGateway(UaString("opc.tcp://192.168.192.117:4840"));
        /*  m_gatewayList[MODULEY] = new OpcuaGateway(UaString("opc.tcp://192.168.192.118:4840"));
          m_gatewayList[MODULEZ] = new OpcuaGateway(UaString("opc.tcp://192.168.192.119:4840"));*/
    }

    m_gatewayList[MANUALMODULE1] = new OpcuaGateway(MAINSERVER);
    m_gatewayList[MAINTENANCEMODULE] = new OpcuaGateway(MAINSERVER);


    //// ENDE ÄNDERN
    m_gatewayList[MODULENUMBERTASK] = new OpcuaGateway(MAINSERVER);
    m_gatewayList[MODULENUMBERMESSAGEFEEDER] = new OpcuaGateway(MAINSERVER);

    if (m_topping_beckhoff_enabled)
    {
        m_gatewayList[MODULENUMBERCREMEBECKHOFF] = new OpcuaGateway(
            UaString("opc.tcp://192.168.192.138:4840"));
    }

    if (m_topping_bosch_enabled)
    {
        m_gatewayList[MODULENUMBERCREMEBOSCH] = new OpcuaGateway(
            UaString("opc.tcp://192.168.192.139:4840"));
    }

    if (m_cocktail_enabled)
    {
        m_gatewayList[MODULENUMBERCOCKTAIL] = new OpcuaGateway(UaString("opc.tcp://192.168.192.121:4840"));
    }

    for (std::map<int, OpcuaGateway*>::iterator it = m_gatewayList.begin(); it != m_gatewayList.end();
         ++it)
    {
        status = it->second->loadConfig(); //TODO - config needed?

        if (!status.isGood())
        {
            QLOG_DEBUG() << "Config load failed." ;
        }

        status = it->second->connect();

        if (!status.isGood())
        {
            QLOG_DEBUG() << "Connection to server of module number " << it->first << " failed." ;
        }

    }

    m_pMessageFeeder = new MessageFeeder(m_gatewayList[MODULENUMBERMESSAGEFEEDER],
                                         MODULENUMBERMESSAGEFEEDER);
    // 2. Stelle ÄNDERN

    m_pMaintenanceHelper = new MaintenanceHelper(m_pMessageFeeder);
    m_initManager = new InitManager();

    if (m_cookie_enabled)
    {
        m_productionModuleList[MODULENUMBERCOOKIESEPARATOR] = new CookieSeparator(
            m_gatewayList[MODULENUMBERCOOKIESEPARATOR], MODULENUMBERCOOKIESEPARATOR, m_pMessageFeeder,
            m_pMaintenanceHelper, m_initManager);
    }

    if (m_xts_enabled)
    {
        m_productionModuleList[MODULENUMBERXTS1] = new Xts(m_gatewayList[MODULENUMBERXTS1],
                MODULENUMBERXTS1, m_pMessageFeeder, m_pMaintenanceHelper, m_initManager);
        m_productionModuleList[MODULENUMBERXTS2] = new Xts(m_gatewayList[MODULENUMBERXTS2],
                MODULENUMBERXTS2, m_pMessageFeeder, m_pMaintenanceHelper, m_initManager);
        m_productionModuleList[MODULENUMBERXTS3] = new Xts(m_gatewayList[MODULENUMBERXTS3],
                MODULENUMBERXTS3, m_pMessageFeeder, m_pMaintenanceHelper, m_initManager);
    }

    if (m_enableInOutput)
    {
        m_productionModuleList[INPUTMODULE] = new ManualProductionModule(m_gatewayList[INPUTMODULE],
                INPUTMODULE, m_pMessageFeeder, m_pMaintenanceHelper, m_initManager);
        m_productionModuleList[OUTPUTMODULE] = new ManualProductionModule(m_gatewayList[OUTPUTMODULE],
                OUTPUTMODULE, m_pMessageFeeder, m_pMaintenanceHelper, m_initManager);

    }

    if (m_virtualModules_enabled)
    {
        m_productionModuleList[MODULEX] = new CookieSeparator(m_gatewayList[MODULEX],
                MODULEX, m_pMessageFeeder, m_pMaintenanceHelper, m_initManager);
        //m_productionModuleList[MODULEY] = new CookieSeparator(m_gatewayList[MODULEY],
        //        MODULEY, m_pMessageFeeder, m_pMaintenanceHelper, m_initManager);
        //m_productionModuleList[MODULEZ] = new CookieSeparator(m_gatewayList[MODULEZ],
        //        MODULEZ, m_pMessageFeeder, m_pMaintenanceHelper, m_initManager);
    }

    // Manual Module

    m_productionModuleList[MANUALMODULE1] = new ManualModule(m_gatewayList[MANUALMODULE1],
            MANUALMODULE1, m_pMessageFeeder);

    m_productionModuleList[MAINTENANCEMODULE] = new ManualModule(m_gatewayList[MAINTENANCEMODULE],
            MAINTENANCEMODULE,
            m_pMessageFeeder);

    if (m_topping_beckhoff_enabled)
    {
        m_productionModuleList[MODULENUMBERCREMEBECKHOFF] = new CremeModule(
            m_gatewayList[MODULENUMBERCREMEBECKHOFF], MODULENUMBERCREMEBECKHOFF, m_pMessageFeeder,
            m_pMaintenanceHelper, m_initManager);
    }

    if (m_topping_bosch_enabled)
    {
        m_productionModuleList[MODULENUMBERCREMEBOSCH] = new CremeModule(
            m_gatewayList[MODULENUMBERCREMEBOSCH], MODULENUMBERCREMEBOSCH, m_pMessageFeeder,
            m_pMaintenanceHelper, m_initManager);
    }

    if (m_cocktail_enabled)
    {
        m_productionModuleList[MODULENUMBERCOCKTAIL] = new CocktailModule(
            m_gatewayList[MODULENUMBERCOCKTAIL], MODULENUMBERCOCKTAIL, m_pMessageFeeder,
            m_pMaintenanceHelper, m_initManager);
    }

    ////ENDE ÄNDERN

    m_taskModule = new TaskModule(m_gatewayList[MODULENUMBERTASK], MODULENUMBERTASK,
                                  m_productionModuleList, m_pMessageFeeder, m_productionModuleList[MANUALMODULE1]);



    m_initManager->setConnections(m_gatewayList, m_productionModuleList, m_pMessageFeeder);
    m_pMaintenanceHelper->setModuleList(m_productionModuleList);

    if (m_simuEnabled)
    {
        m_simuFeeder = new SimulationFeeder(m_gatewayList[MODULENUMBERSIMULATIONFEEDER],
                                            MODULENUMBERSIMULATIONFEEDER, m_productionModuleList, m_pMessageFeeder);

    }

    return status;
}

void ProcessHandler::run()
{
    UaStatus status;

    /*
    ** Startup all production modules.
    */
    for (std::map<int, IProductionModule*>::iterator it = m_productionModuleList.begin();
         it != m_productionModuleList.end(); ++it)
    {
        it->second->startup();
    }

    /*
    ** Startup init module and initialize the positions.
    */
    QLOG_DEBUG() << "Startup finished." ;
    m_pMessageFeeder->write(UaString("Startup finished"), msgSuccess);

    buildSkillList();

    if (m_init)
    {
        int calibrationStatus = m_initManager->startUpSystem();

    }




    //getchar();

    /*
    ** Start the task module.
    */

    m_taskModule->startup();
}

void ProcessHandler::buildSkillList()
{
    for (std::map<int, IProductionModule*>::iterator it = m_productionModuleList.begin();
         it != m_productionModuleList.end(); ++it)
    {
        m_moduleSkillList.insert(std::pair<int, std::map<int, int>>(it->first,
                                 it->second->getSkills())); //The getskills() call actually triggers the build of modules' skill lists.
        QLOG_DEBUG() << "Active Module: " << it->second->getModuleName().toUtf8() << ", Nr. #" << it->first;
    }

    QLOG_DEBUG() << "\nAvailable skills:" ;

    for (std::multimap<int, std::map<int, int>>::iterator it = m_moduleSkillList.begin();
         it != m_moduleSkillList.end(); ++it)
    {
        QLOG_DEBUG() << "=============================" ;
        QLOG_DEBUG() << "Module number: " << it->first ;

        for (std::map<int, int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            QLOG_DEBUG() <<  "Skill ID: " << it2->first <<
                         ", Skill OPC UA identifier (in output interface): SkillOutput"
                         <<
                         it2->second ;
        }
    }

    QLOG_DEBUG() << "=======End of Skill list=========" ;

}
