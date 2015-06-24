#ifndef JAVASCRIPTMODULE_H
#define JAVASCRIPTMODULE_H


#include <Mi5_ProcessTool/include/ProductionModule.h>

//! JavaScriptModule base class.
/*!
    Digital representation of the JavaScriptModule production module.
*/
class JavaScriptModule : public ProductionModule
{

public:
    JavaScriptModule(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
                     MaintenanceHelper* pHelper, InitManager* pInitManager, int skillCount = 16);
    ~JavaScriptModule();

protected slots:
    virtual void evaluateError();

};

#endif // JAVASCRIPTMODULE_H