#include <Mi5_ProcessTool/include/ProductionModules/CocktailModule.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

CocktailModule::CocktailModule(OpcuaGateway* pOpcuaGateway,
                               int moduleNumber, MessageFeeder* pMessageFeeder,
                               MaintenanceHelper* pHelper) : ProductionModule(pOpcuaGateway, moduleNumber,
                                           pMessageFeeder, pHelper)
{
    QLOG_DEBUG() << "Created module CocktailModule with module number " << moduleNumber ;
}

CocktailModule::~CocktailModule()
{
}

void CocktailModule::evaluateError()
{

}
