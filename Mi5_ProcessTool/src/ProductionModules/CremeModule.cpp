#include <Mi5_ProcessTool/include/ProductionModules/CremeModule.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

CremeModule::CremeModule(OpcuaGateway* pOpcuaGateway,
                         int moduleNumber, MessageFeeder* pMessageFeeder) : ProductionModule(pOpcuaGateway, moduleNumber,
                                     pMessageFeeder)
{
    QLOG_DEBUG() << "Created module CremeModule with module number " << moduleNumber ;
}

CremeModule::~CremeModule()
{
}