#ifndef XTS_H
#define XTS_H

#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

class Xts : public ProductionModule
{

public:
    Xts(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder);
    ~Xts();

};

#endif // XTS_H