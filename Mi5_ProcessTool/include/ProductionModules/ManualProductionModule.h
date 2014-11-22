#ifndef MANUALPRODUCTIONMODULE_H
#define MANUALPRODUCTIONMODULE_H
#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

class ManualProductionModule : public ProductionModule
{
    Q_OBJECT
public:
    ManualProductionModule(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
                           MaintenanceHelper* pHelper, InitManager* pInitManager);
    ~ManualProductionModule();

protected slots:
    virtual void evaluateError();


};

#endif // MANUALPRODUCTIONMODULE_H