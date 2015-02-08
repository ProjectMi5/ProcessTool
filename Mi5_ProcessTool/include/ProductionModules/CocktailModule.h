#ifndef COCKTAILMODULE_H
#define COCKTAILMODULE_H
#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

//! Cocktail production module.
/*!
    Digital representation of the cocktail production module.
*/
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