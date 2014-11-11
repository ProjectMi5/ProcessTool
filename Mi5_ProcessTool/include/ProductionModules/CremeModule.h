#ifndef CREMEMODULE_H
#define CREMEMODULE_H
#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

class CremeModule : public ProductionModule
{

public:
    CremeModule(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder);
    ~CremeModule();

};

#endif // CREMEMODULE_H