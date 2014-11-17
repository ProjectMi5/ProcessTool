#include <Mi5_ProcessTool/include/ProductionModules/CremeModule.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

CremeModule::CremeModule(OpcuaGateway* pOpcuaGateway,
                         int moduleNumber, MessageFeeder* pMessageFeeder,
                         MaintenanceHelper* pHelper) : ProductionModule(pOpcuaGateway, moduleNumber,
                                     pMessageFeeder, pHelper)
{
    QLOG_DEBUG() << "Created module CremeModule with module number " << moduleNumber ;
}

CremeModule::~CremeModule()
{
}

void CremeModule::evaluateError()
{

}
