#include <Mi5_ProcessTool/include/CookieSeparator.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <Mi5_ProcessTool/include/MaintenanceHelper.h>

CookieSeparator::CookieSeparator(OpcuaGateway* pOpcuaGateway,
                                 int moduleNumber, MessageFeeder* pMessageFeeder,
                                 MaintenanceHelper* pHelper, InitManager* pInitManager) : ProductionModule(pOpcuaGateway,
                                             moduleNumber,
                                             pMessageFeeder, pHelper, pInitManager)
{
    QLOG_DEBUG() << "Created module CookieSeparator with module number " << moduleNumber ;
}

CookieSeparator::~CookieSeparator()
{
}

void CookieSeparator::evaluateError()
{
    int errorId = getErrorId();

    if ((bool)output.error == true && errorId > 0)
    {
        if (errorId == MODULECOOKIEREFILLERRORID)
        {
            m_pMaintenanceHelper->maintain(m_moduleNumber, errorId);
        }
    }
    else
    {
        QLOG_DEBUG() << "Received error, but no error id at module " << m_moduleNumber << ".";
    }
}