#include <Mi5_ProcessTool/include/CookieSeparator.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

CookieSeparator::CookieSeparator(OpcuaGateway* pOpcuaGateway,
                                 int moduleNumber, MessageFeeder* pMessageFeeder) : ProductionModule(pOpcuaGateway, moduleNumber,
                                             pMessageFeeder)
{
    QLOG_DEBUG() << "Created module CookieSeparator with module number " << moduleNumber ;
}

CookieSeparator::~CookieSeparator()
{
}