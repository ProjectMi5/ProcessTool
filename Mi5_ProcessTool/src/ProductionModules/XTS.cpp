#include <Mi5_ProcessTool/include/ProductionModules/XTS.h>

Xts::Xts(OpcuaGateway* pOpcuaGateway,
         int moduleNumber, MessageFeeder* pMessageFeeder) : ProductionModule(pOpcuaGateway, moduleNumber,
                     pMessageFeeder)
{
    std::cout << "Created module XTS with module number " << moduleNumber << std::endl;
}

Xts::~Xts()
{

}