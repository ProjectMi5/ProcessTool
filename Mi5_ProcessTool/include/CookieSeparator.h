#ifndef COOKIESEPARATOR_H
#define COOKIESEPARATOR_H
#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

class CookieSeparator : public ProductionModule
{

public:
    CookieSeparator(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder);
    ~CookieSeparator();

};

#endif // COOKIESEPARATOR_H