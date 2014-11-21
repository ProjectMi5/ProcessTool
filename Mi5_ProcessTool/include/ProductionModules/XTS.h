#ifndef XTS_H
#define XTS_H

#include <iostream>

#include <Mi5_ProcessTool/include/ProductionModule.h>

class Xts : public ProductionModule
{

public:
    Xts(OpcuaGateway* pOpcuaGateway, int moduleNumber, MessageFeeder* pMessageFeeder,
        MaintenanceHelper* pHelper, InitManager* pInitManager);
    ~Xts();

public:
    virtual bool isBlocked();
    virtual bool isReserved();

private:
    virtual void checkMoverState(int skillPos);

private:
    bool m_blocked;
    bool m_reserved;

protected slots:
    virtual void evaluateError();

};

#endif // XTS_H