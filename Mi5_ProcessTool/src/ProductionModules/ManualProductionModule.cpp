#include <Mi5_ProcessTool/include/ProductionModules/ManualProductionModule.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

ManualProductionModule::ManualProductionModule(OpcuaGateway* pOpcuaGateway,
        int moduleNumber, MessageFeeder* pMessageFeeder,
        MaintenanceHelper* pHelper, InitManager* pInitManager) : ProductionModule(pOpcuaGateway,
                    moduleNumber,
                    pMessageFeeder, pHelper, pInitManager)
{
    QLOG_DEBUG() << "Created module ManualProductionModule with module number " << moduleNumber ;
    m_enableConnectionTest = false;
}

ManualProductionModule::~ManualProductionModule()
{
}

void ManualProductionModule::evaluateError()
{

}
