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
    ManualModule* m_manualModule;
    QThread m_thread;
    MessageFeeder* m_pMessageFeeder;

private:
    UaStatus build();
    UaStatus loadConfig();
    void run();
    void buildSkillList();
    void initialInit();

};


#endif // PROCESSHANDLER_H