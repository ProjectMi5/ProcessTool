#ifndef PROCESSHANDLER_H
#define PROCESSHANDLER_H

#include <QThread>

#include <uaplatformlayer.h>
#include <iostream>
#include <Mi5_ProcessTool/include/UaHelper.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <Mi5_ProcessTool/include/CookieSeparator.h>
#include <Mi5_ProcessTool/include/ProductionModules/CremeModule.h>
#include <Mi5_ProcessTool/include/ProductionModules/XTS.h>
#include <Mi5_ProcessTool/include/TaskModule.h>
#include <Mi5_ProcessTool/include/InitManager.h>
#include <Mi5_ProcessTool/include/GlobalConsts.h>
#include <Mi5_ProcessTool/include/ProductionModules/ManualModule.h>
#include <Mi5_ProcessTool/include/MaintenanceHelper.h>
#include <Mi5_ProcessTool/include/ProductionModules/CocktailModule.h>
#include <Mi5_ProcessTool/include/Synchronization/SimulationFeeder.h>
#include <Mi5_ProcessTool/include/ProductionModules/ManualProductionModule.h>

//! Main class and entry point for the Mi5 Process Tool.
/*!
    In this class, the given configuration is read in and the systems boots up according to it.
*/
class ProcessHandler
{
public:
    ProcessHandler(bool initialInit);
    ~ProcessHandler();
    void start();

private:
    Xts* m_xts;
    CremeModule* m_cremeModule;
    TaskModule* m_taskModule;
    InitManager* m_initManager;
    std::multimap<int, std::map<int, int>> m_moduleSkillList;
    std::map<int, OpcuaGateway*> m_gatewayList;
    std::map<int, IProductionModule*> m_productionModuleList;
    QThread m_thread;
    MessageFeeder* m_pMessageFeeder;
    MaintenanceHelper* m_pMaintenanceHelper;
    SimulationFeeder* m_simuFeeder;
    bool m_xts_enabled;
    bool m_cookie_enabled;
    bool m_topping_beckhoff_enabled;
    bool m_topping_bosch_enabled;
    bool m_cocktail_enabled;
    bool m_virtualModules_enabled;
    bool m_init;
    bool m_simuEnabled;
    bool m_enableInOutput;
private:
    UaStatus build();
    UaStatus loadConfig();
    void run();
    void buildSkillList();

};


#endif // PROCESSHANDLER_H