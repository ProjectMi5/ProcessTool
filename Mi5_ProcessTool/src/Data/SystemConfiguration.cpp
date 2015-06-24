#include <Mi5_ProcessTool/include/SystemConfiguration.h>

ModuleConfiguration::ModuleConfiguration(QString Name, QString Ip,
        bool Enable) : name(Name), ip(Ip), enable(Enable)
{
}

ModuleConfiguration::~ModuleConfiguration()
{
}

SystemConfiguration::SystemConfiguration()
{
    moduleList.clear();
    ModuleConfiguration xts("xts", "", false);
    moduleList.append(xts);
    ModuleConfiguration cookie("cookie", "", false);
    moduleList.append(cookie);
    ModuleConfiguration toppingBeckhoff("toppingBeckhoff", "", false);
    moduleList.append(toppingBeckhoff);
    ModuleConfiguration toppingBosch("toppingBosch", "", false);
    moduleList.append(toppingBosch);
    ModuleConfiguration cocktail("cocktail", "", false);
    moduleList.append(cocktail);
    ModuleConfiguration virtualModule("virtualModules", "", false);
    moduleList.append(virtualModule);
    ModuleConfiguration simulation("simu", "", false);
    moduleList.append(simulation);
    ModuleConfiguration inputOutput("inputOutput", "", false);
    moduleList.append(inputOutput);
    ModuleConfiguration cupDispenser("cupDispenser", "", false);
    moduleList.append(cupDispenser);
}

SystemConfiguration::~SystemConfiguration()
{
}