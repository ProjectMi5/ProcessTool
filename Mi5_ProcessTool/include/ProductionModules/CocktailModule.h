#ifndef COCKTAILMODULE_H
#define COCKTAILMODULE_H
#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

class CocktailModule : public ProductionModule
{

public:
    CocktailModule(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
                   MaintenanceHelper* pHelper, InitManager* pInitManager);
    ~CocktailModule();

protected slots:
    virtual void evaluateError();

};

#endif // COCKTAILMODULE_H