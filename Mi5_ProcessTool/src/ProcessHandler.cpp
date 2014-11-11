#include <Mi5_ProcessTool/include/ProcessHandler.h>

ProcessHandler::ProcessHandler()
{
    m_xts = NULL;
    m_cremeModule = NULL;
    m_taskModule = NULL;

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
    std::cout << "Starting MI5 Process Tool.." << std::endl;
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
    // 1. Stelle ÄNDERN
    m_gatewayList[MODULENUMBERXTS1] = new OpcuaGateway(UaString("opc.tcp://192.168.175.210:4840"));
    m_gatewayList[MODULENUMBERXTS2] = new OpcuaGateway(UaString("opc.tcp://192.168.175.210:4840"));
    m_gatewayList[MODULENUMBERXTS3] = new OpcuaGateway(UaString("opc.tcp://192.168.175.210:4840"));

    m_gatewayList[MODULEX] = new OpcuaGateway(UaString("opc.tcp://192.168.175.224:4840"));
    m_gatewayList[MODULEY] = new OpcuaGateway(UaString("opc.tcp://192.168.175.225:4840"));
    m_gatewayList[MODULEZ] = new OpcuaGateway(UaString("opc.tcp://192.168.175.226:4840"));

    m_gatewayList[MANUALMODULE1] = new OpcuaGateway(UaString("opc.tcp://192.168.175.230:4840"));
    // ENDE ÄNDERN
    m_gatewayList[MODULENUMBERTASK] = new OpcuaGateway(UaString("opc.tcp://192.168.175.230:4840"));
    m_gatewayList[MODULENUMBERMESSAGEFEEDER] = new OpcuaGateway(
        UaString("opc.tcp://192.168.175.230:4840"));

    for (std::map<int, OpcuaGateway*>::iterator it = m_gatewayList.begin(); it != m_gatewayList.end();
         ++it)
    {
        status = it->second->loadConfig(); //TODO - config needed?

        if (!status.isGood())
        {
            std::cout << "Config load failed." << std::endl;
            return status;
        }

        status = it->second->connect();

        if (!status.isGood())
        {
            std::cout << "Connection to server failed." << std::endl;
            return status;
        }

    }

    m_pMessageFeeder = new MessageFeeder(m_gatewayList[MODULENUMBERMESSAGEFEEDER],
                                         MODULENUMBERMESSAGEFEEDER);
    // 2. Stelle ÄNDERN
    m_productionModuleList[MODULENUMBERXTS1] = new Xts(m_gatewayList[MODULENUMBERXTS1],
            MODULENUMBERXTS1, m_pMessageFeeder);
    m_productionModuleList[MODULENUMBERXTS2] = new Xts(m_gatewayList[MODULENUMBERXTS2],
            MODULENUMBERXTS2, m_pMessageFeeder);
    m_productionModuleList[MODULENUMBERXTS3] = new Xts(m_gatewayList[MODULENUMBERXTS3],
            MODULENUMBERXTS3, m_pMessageFeeder);

    m_productionModuleList[MODULEX] = new CookieSeparator(m_gatewayList[MODULEX],
            MODULEX, m_pMessageFeeder);
    m_productionModuleList[MODULEY] = new CookieSeparator(m_gatewayList[MODULEY],
            MODULEY, m_pMessageFeeder);
    m_productionModuleList[MODULEZ] = new CookieSeparator(m_gatewayList[MODULEZ],
            MODULEZ, m_pMessageFeeder);

    // Manual Module
    m_manualModule = new ManualModule(m_gatewayList[MANUALMODULE1],
                                      MANUALMODULE1, m_pMessageFeeder);

    //ENDE ÄNDERN

    //m_xts = new Xts(m_pOpcuaGateway, MODULENUMBERXTS);
    //m_cremeModule = new CremeModule(m_pOpcuaGateway, MODULENUMBERCREME);
    m_taskModule = new TaskModule(m_gatewayList[MODULENUMBERTASK], MODULENUMBERTASK,
                                  m_productionModuleList, m_pMessageFeeder, m_manualModule);


    m_initModule = new InitModule(m_gatewayList, m_productionModuleList);

    return status;
}

void ProcessHandler::run()
{
    UaStatus status;

    for (std::map<int, IProductionModule*>::iterator it = m_productionModuleList.begin();
         it != m_productionModuleList.end(); ++it)
    {
        it->second->startup(); //Startup all production modules.
    }

    m_manualModule->startup();
    m_initModule->startup();
    std::cout << "Startup finished." << std::endl;
    m_pMessageFeeder->write(UaString("Startup finished"), msgSuccess);


    // Nur eine kosmetische Ausgabe.
    buildSkillList();
    //initialInit();
    //getchar();

    m_taskModule->startup();

}

void ProcessHandler::buildSkillList() //obsolete here.
{
    for (std::map<int, IProductionModule*>::iterator it = m_productionModuleList.begin();
         it != m_productionModuleList.end(); ++it)
    {
        m_moduleSkillList.insert(std::pair<int, std::map<int, int>>(it->first, it->second->getSkills()));
    }

    std::cout << "\nAvailable skills:" << std::endl;

    for (std::multimap<int, std::map<int, int>>::iterator it = m_moduleSkillList.begin();
         it != m_moduleSkillList.end(); ++it)
    {
        std::cout << "=============================" << std::endl;
        std::cout << "Module number: " << it->first << std::endl;

        for (std::map<int, int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            std:: cout <<  "Skill ID: " << it2->first <<
                       ", Skill OPC UA identifier (in output interface): SkillOutput"
                       <<
                       it2->second << std::endl;
        }
    }

    std::cout << "=======End of Skill list=========" << std::endl;

}

void ProcessHandler::initialInit()
{
    //for (std::map<int, IProductionModule*>::iterator it = m_productionModuleList.begin();
    //     it != m_productionModuleList.end(); ++it)
    //{
    //    m_initModule->positionCalibration(it->first);
    //}
    //Dont init the XTS modules!
    m_initModule->positionCalibration(MODULEX);
    m_initModule->positionCalibration(MODULEY);
    m_initModule->positionCalibration(MODULEZ);


    m_pMessageFeeder->write("Initialization of the production modules done.", msgSuccess);
}
