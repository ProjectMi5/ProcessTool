#ifndef COOKIESEPARATOR_H
#define COOKIESEPARATOR_H
#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

//! CookieSeparator production module.
/*!
    Digital representation of the CookieSeparator production module.
*/
class CookieSeparator : public ProductionModule
{
    Q_OBJECT
public:
    CookieSeparator(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
                    MaintenanceHelper* pHelper, InitManager* pInitManager);
    ~CookieSeparator();

protected slots:
    virtual void evaluateError();


};

#endif // COOKIESEPARATOR_H