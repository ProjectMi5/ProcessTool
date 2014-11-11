#include <Mi5_ProcessTool/include/CookieSeparator.h>

CookieSeparator::CookieSeparator(OpcuaGateway* pOpcuaGateway,
                                 int moduleNumber, MessageFeeder* pMessageFeeder) : ProductionModule(pOpcuaGateway, moduleNumber,
                                             pMessageFeeder)
{
    std::cout << "Created module CookieSeparator with module number " << moduleNumber << std::endl;
}

CookieSeparator::~CookieSeparator()
{
}