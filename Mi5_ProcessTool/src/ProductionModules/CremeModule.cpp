#include <Mi5_ProcessTool/include/ProductionModules/CremeModule.h>

CremeModule::CremeModule(OpcuaGateway* pOpcuaGateway,
                         int moduleNumber, MessageFeeder* pMessageFeeder) : ProductionModule(pOpcuaGateway, moduleNumber,
                                     pMessageFeeder)
{
    std::cout << "Created module CremeModule with module number " << moduleNumber << std::endl;
}

CremeModule::~CremeModule()
{
}