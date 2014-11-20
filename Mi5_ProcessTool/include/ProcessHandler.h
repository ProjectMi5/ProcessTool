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
#include <Mi5_ProcessTool/include/InitModule.h>
#include <Mi5_ProcessTool/include/GlobalConsts.h>
#include <Mi5_ProcessTool/include/ProductionModules/ManualModule.h>
#include <Mi5_ProcessTool/include/MaintenanceHelper.h>
#include <Mi5_ProcessTool/include/ProductionModules/CocktailModule.h>

class ProcessHandler
{
public:
    ProcessHandler();
    ~ProcessHandler();
    void start();

private:
    Xts* m_xts;
    CremeModule* m_cremeModule;
    TaskModule* m_taskModule;
    InitModule* m_initModule;
    std::multimap<int, std::map<int, int>> m_moduleSkillList;
    std::map<int, OpcuaGateway*> m_gatewayList;
    std::map<int, IProductionModule*> m_productionModuleList;
    QThread m_thread;
    MessageFeeder* m_pMessageFeeder;
    MaintenanceHelper* m_pMaintenanceHelper;
    bool m_xts_enabled;
    bool m_cookie_enabled;
    bool m_topping_beckhoff_enabled;
    bool m_topping_bosch_enabled;
    bool m_cocktail_enabled;
    bool m_virtualModules_enabled;
    bool m_init;

private:
    UaStatus build();
    UaStatus loadConfig();
    void run();
    void buildSkillList();
    void initialInit();

};


#endif // PROCESSHANDLER_H