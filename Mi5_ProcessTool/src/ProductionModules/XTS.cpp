#include <Mi5_ProcessTool/include/ProductionModules/XTS.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>

Xts::Xts(OpcuaGateway* pOpcuaGateway,
         int moduleNumber, MessageFeeder* pMessageFeeder) : ProductionModule(pOpcuaGateway, moduleNumber,
                     pMessageFeeder), m_reserved(false), m_blocked(false)
{
    QLOG_DEBUG() << "Created module XTS with module number " << moduleNumber ;
}

Xts::~Xts()
{

}

bool Xts::isBlocked()
{
    return m_blocked;
}

bool Xts::isReserved()
{
    return m_reserved;
}


void Xts::checkMoverState(int skillPos)
{
    int skillId = output.skillOutput[skillPos].id;

    switch (skillId)
    {
    case SKILLIDXTSRESERVE:
        m_reserved = true;
        break;

    case SKILLIDXTSRELEASE:
        m_reserved = false;
        break;

    case SKILLIDXTSBLOCK:
        m_blocked = true;
        break;

    case SKILLIDXTSUNBLOCK:
        m_blocked = false;
        break;

    default:
        break;
    }
}