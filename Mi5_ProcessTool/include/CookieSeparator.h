#ifndef COOKIESEPARATOR_H
#define COOKIESEPARATOR_H
#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

class CookieSeparator : public ProductionModule
{
    Q_OBJECT
public:
    CookieSeparator(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
                    MaintenanceHelper* pHelper);
    ~CookieSeparator();

protected slots:
    virtual void evaluateError();


};

#endif // COOKIESEPARATOR_H