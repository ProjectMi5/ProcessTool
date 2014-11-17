#include <Mi5_ProcessTool/include/MaintenanceHelper.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>


MaintenanceHelper::MaintenanceHelper() :
    m_moduleToMaintain(-1),
    m_maintenanceInProcess(false)
{
    moveToThread(&m_thread);
    m_thread.start();
}

MaintenanceHelper::~MaintenanceHelper()
{

}

void MaintenanceHelper::setModuleList(std::map<int, IProductionModule*> pModuleList)
{
    m_pModuleList = pModuleList;
}

void MaintenanceHelper::maintain(int moduleNumber, int errorId)
{
    m_moduleToMaintain = moduleNumber;

    if (m_pModuleList.count(moduleNumber) > 0)
    {
        int skillId = MANUALUNIVERSALSKILL; //Todo: Fix this mess.
        bool maintenanceModuleReady = false;

        while (!maintenanceModuleReady)
        {
            int moduleState = m_pModuleList[MAINTENANCEMODULE]->checkSkillState(skillId);

            if (moduleState == SKILLMODULEREADY)
            {
                QLOG_DEBUG() << "Maintaining module " << moduleNumber;
                maintenanceExecution(moduleNumber, errorId);

                // Wait til its done.
                m_mutex.lock();
                m_waitCondition.wait(&m_mutex);
                maintenanceModuleReady = true;
                m_mutex.unlock();
            }
        }

    }
    else
    {
        QLOG_ERROR() << "MaintenanceHelper received unknown module number " << moduleNumber ;
    }

}

void MaintenanceHelper::subscriptionDataChange(OpcUa_UInt32 clientSubscriptionHandle,
        const UaDataNotifications& dataNotifications, const UaDiagnosticInfos& diagnosticInfos)
{

}

void MaintenanceHelper::startup()
{

}

void MaintenanceHelper::serverReconnected()
{

}

int MaintenanceHelper::getTaskId()
{
    return 1337;
}

void MaintenanceHelper::skillStateChanged(int moduleNumber, int skillPos, int state)
{
    //if (m_calibrationInProgress &&
    //    (skillPos == m_pModuleList[moduleNumber]->translateSkillIdToSkillPos(POSCALSKILLID)) &&
    //    (moduleNumber == m_usedXtsModuleNumber))
    if (1)
    {
        QLOG_DEBUG() << "Statechange: " << state ;

        switch (state)
        {

        case SKILLMODULEERROR:
            break;

        case SKILLMODULEDONE:
            QLOG_DEBUG() << "Maintenance done." ;
            resetData();
            break;

        case SKILLMODULEBUSY:
            QLOG_DEBUG() << "Maintenance in progress." ;
            break;

        default:
            break;
        }
    }
}

void MaintenanceHelper::resetData()
{
    int skillPos = m_pModuleList[MAINTENANCEMODULE]->translateSkillIdToSkillPos(MANUALUNIVERSALSKILL);
    m_pModuleList[MAINTENANCEMODULE]->deregisterTaskForSkill(skillPos);

    m_pModuleList[m_moduleToMaintain]->changeModuleMode(ModuleModeAuto);
    m_waitCondition.wakeAll();
    m_moduleToMaintain = -1;
    m_maintenanceInProcess = false;
}

void MaintenanceHelper::maintenanceExecution(int moduleNumber, int errorId)
{
    int skillpos = m_pModuleList[MAINTENANCEMODULE]->translateSkillIdToSkillPos(MANUALUNIVERSALSKILL);
    m_pModuleList[moduleNumber]->changeModuleMode(ModuleModeManual);
    m_maintenanceInProcess = true;
    m_pModuleList[MAINTENANCEMODULE]->registerTaskForSkill(this, skillpos);

    ParameterInputArray tmpParamArray;

    for (int i = 0; i < PARAMETERCOUNT; i++)
    {
        tmpParamArray.paramInput[i].value = 0;
        tmpParamArray.paramInput[i].string = "not used";
    }

    m_pModuleList[MAINTENANCEMODULE]->executeSkill(skillpos, tmpParamArray);
}