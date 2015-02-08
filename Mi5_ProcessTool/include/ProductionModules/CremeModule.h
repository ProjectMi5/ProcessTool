#ifndef CREMEMODULE_H
#define CREMEMODULE_H
#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

//! CremeModule production module.
/*!
    Digital representation of the CremeModule production module.
*/
class CremeModule : public ProductionModule
{

public:
    CremeModule(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
                MaintenanceHelper* pHelper, InitManager* pInitManager);
    ~CremeModule();

protected slots:
    virtual void evaluateError();

};

#endif // CREMEMODULE_H