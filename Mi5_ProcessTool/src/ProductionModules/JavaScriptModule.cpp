#include <Mi5_ProcessTool/include/ProductionModules/JavaScriptModule.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

JavaScriptModule::JavaScriptModule(OpcuaGateway* pOpcuaGateway,
                                   int moduleNumber, MessageFeeder* pMessageFeeder,
                                   MaintenanceHelper* pHelper, InitManager* pInitManager,
                                   int skillCount) : ProductionModule(pOpcuaGateway,
                                               moduleNumber,
                                               pMessageFeeder, pHelper, pInitManager, skillCount)
{
    m_enableConnectionTest = false;
    QLOG_DEBUG() << "Created module JavaScriptModule with module number " << moduleNumber ;
}

JavaScriptModule::~JavaScriptModule()
{
}

void JavaScriptModule::evaluateError()
{

}
