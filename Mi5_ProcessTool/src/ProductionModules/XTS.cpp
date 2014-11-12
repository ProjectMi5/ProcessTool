#include <Mi5_ProcessTool/include/ProductionModules/XTS.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

Xts::Xts(OpcuaGateway* pOpcuaGateway,
         int moduleNumber, MessageFeeder* pMessageFeeder) : ProductionModule(pOpcuaGateway, moduleNumber,
                     pMessageFeeder)
{
    QLOG_DEBUG() << "Created module XTS with module number " << moduleNumber ;
}

Xts::~Xts()
{

}