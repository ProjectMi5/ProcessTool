#include <Mi5_ProcessTool/include/InitManager.h>



InitManager::InitManager(bool initialInit) : m_positionCalibrator(NULL), m_initialInitDone(false),
    m_initialInit(initialInit)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(evalInitDemands()));

    moveToThread(&m_thread);
    m_thread.setObjectName("TaskInitManager");
    m_thread.start();
}

InitManager::~InitManager()
{
    m_positionCalibrator->deleteLater();
    m_positionCalibrator = NULL;
}

int InitManager::enqueueForInit(int moduleNumber)
{
    int returnVal = -1;

    if (std::find(m_initQueue.begin(), m_initQueue.end(), moduleNumber) == m_initQueue.end())
    {
        m_initQueue.push_back(moduleNumber);
    }
    else
    {
        QLOG_ERROR() << "Already enqueued module number " << moduleNumber <<
                     " for initialization.";
    }

    return returnVal;
}

int InitManager::startUpSystem()
{
    int returnVal = -1;

    if (m_initialInit)
    {
        if (m_positionCalibrator != NULL)
        {
            evalInitDemands();
            returnVal = 1;
            //m_timer->start(15000);
        }
        else
        {
            QLOG_ERROR() << "Setup connections first.";
        }
    }
    else
    {
        m_initQueue.clear();
    }

    return returnVal;
}

void InitManager::evalInitDemands() //Only call this method from within m_thread!
{
    std::vector<int> initQueueCopy = m_initQueue;
    int returnVal;

    for (std::vector<int>::iterator it =  initQueueCopy.begin(); it != initQueueCopy.end(); it++)
    {
        returnVal = m_positionCalibrator->positionCalibration(*it);

        if (returnVal == 1)
        {
            m_initQueue.erase(std::find(m_initQueue.begin(), m_initQueue.end(), *it));
        }
    }

    if (m_initQueue.size() == 0)
    {
        m_initialInitDone = true;
    }
}

void InitManager::setConnections(std::map<int, OpcuaGateway*> pGatewayList,
                                 std::map<int, IProductionModule*> pModuleList, MessageFeeder* pMsgFeeder)
{
    m_pGatewayList = pGatewayList;
    m_pModuleList = pModuleList;
    m_pMsgFeeder = pMsgFeeder;

    m_positionCalibrator = new PositionCalibrator(pGatewayList, pModuleList, pMsgFeeder);
    m_positionCalibrator->startup();
}

bool InitManager::isInitialInitDone()
{
    return m_initialInitDone;
}
