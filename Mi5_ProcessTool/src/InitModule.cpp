#include <Mi5_ProcessTool/include/InitModule.h>
#include <Mi5_ProcessTool/include/OpcuaGateway.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
InitModule::InitModule(std::map<int, OpcuaGateway*> pGatewayList,
                       std::map<int, IProductionModule*> pModuleList)
{
    m_xtsModuleNumbers.clear();
    m_pGatewayList.clear();
    m_pModuleList.clear();
    m_calibrationInProgress = false;
    m_moduleToCalibrate = -1;
    m_usedXtsModuleNumber = -1;

    m_pGatewayList = pGatewayList;
    m_pModuleList = pModuleList;

    moveToThread(&m_thread);
    m_thread.start();
}

InitModule::~InitModule()
{
}

void InitModule::startup()
{
    int xtsModulesCount = evalModuleList();
    QLOG_DEBUG() << "Initmodule found " << xtsModulesCount << " XTS module(s)." ;
}

void InitModule::serverReconnected()
{

}

void InitModule::subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
                                        const UaDataNotifications& dataNotifications,
                                        const UaDiagnosticInfos&   diagnosticInfos)
{
}

int InitModule::positionCalibration(int moduleNumber)
{
    m_moduleToCalibrate = moduleNumber;
    int returnval = -1;

    if (m_pModuleList.count(moduleNumber) > 0)
    {
        for (std::vector<int>::iterator it = m_xtsModuleNumbers.begin(); it != m_xtsModuleNumbers.end();
             it++)
        {
            int skillId = POSCALSKILLID; //Todo: Fix this mess.
            int moduleState = m_pModuleList[*it]->checkSkillState(skillId);

            if (moduleState == SKILLMODULEREADY)
            {
                QLOG_DEBUG() << "Calibrating module " << moduleNumber << " with transport module " << *it ;
                positionCalibrationExecution(moduleNumber, *it);

                // Wait til its done.
                m_mutex.lock();
                m_waitCondition.wait(&m_mutex);
                returnval = 1;
                m_mutex.unlock();
                break;
            }
            else
            {
                //TODO: wait for a skill to become ready
                QLOG_ERROR() << "Found no ready XTS module for position calibration of module number " <<
                             moduleNumber
                             ;
            }
        }
    }
    else
    {
        QLOG_ERROR() << "InitModule received unknown module number " << moduleNumber ;
    }

    return returnval;
}

int InitModule::evalModuleList()
{
    m_xtsModuleNumbers.clear();

    if (!m_pModuleList.empty())
    {
        for (int i = MODULENUMBERXTSMIN; i <= MODULENUMBERXTSMAX; i++)
        {
            if (m_pModuleList.count(i) > 0)
            {
                m_xtsModuleNumbers.push_back(i);
            }
        }
    }

    return m_xtsModuleNumbers.size();
}

void InitModule::positionCalibrationExecution(int moduleNumber, int xtsModuleNumber)
{
    m_usedXtsModuleNumber = xtsModuleNumber;
    // We already checked, that the module's skill is ready.
    IProductionModule* pModule = m_pModuleList[xtsModuleNumber];
    int skillPos = pModule->translateSkillIdToSkillPos(POSCALSKILLID);
    int status = pModule->registerTaskForSkill(this, skillPos);

    if (status == 0)
    {
        m_calibrationInProgress = true;
        // now: execute the skill with the appropriate params.

        //execute this skill on the correspondent module
        ParameterInputArray tmpParamArray;
        UaString tmpString;
        tmpParamArray.paramInput[0].value = 0;
        tmpParamArray.paramInput[0].string = m_pGatewayList[moduleNumber]->getServerUrl();

        QString tmpQString = m_pModuleList[m_moduleToCalibrate]->getBaseNodeId().toUtf8();
        QStringList tmpStringList = tmpQString.split("=");//ns=4;s=asdf
        tmpString = UaString(tmpStringList[2].toUtf8());
        tmpString += ".Output.PositionSensor";
        tmpParamArray.paramInput[1].value = 0;
        tmpParamArray.paramInput[1].string = tmpString;

        tmpString = UaString(tmpStringList[2].toUtf8());
        tmpString += ".Input.PositionInput";
        tmpParamArray.paramInput[2].value = 0;
        tmpParamArray.paramInput[2].string = tmpString;

        QStringList tmpStringList2 = tmpStringList[1].split(";");
        tmpString = UaString(tmpStringList2[0].toUtf8());
        tmpParamArray.paramInput[3].value = 0;
        tmpParamArray.paramInput[3].string = tmpString;

        for (int i = 4; i < PARAMETERCOUNT; i++)
        {
            tmpParamArray.paramInput[i].value = 0;
            tmpParamArray.paramInput[i].string = "not used";
        }

        pModule->executeSkill(skillPos, tmpParamArray);
    }
    else
    {
        // doh.
    }
}

int InitModule::getTaskId()
{
    return 1337;
}

void InitModule::skillStateChanged(int moduleNumber, int skillPos, int state)
{
    if (m_calibrationInProgress &&
        (skillPos == m_pModuleList[moduleNumber]->translateSkillIdToSkillPos(POSCALSKILLID)) &&
        (moduleNumber == m_usedXtsModuleNumber))
    {
        QLOG_DEBUG() << "Statechange: " << state ;

        switch (state)
        {

        case SKILLMODULEERROR:
            break;

        case SKILLMODULEDONE:
            QLOG_DEBUG() << "Calibration done." ;
            resetData();
            break;

        case SKILLMODULEBUSY:
            QLOG_DEBUG() << "Calibration in progress." ;
            break;

        default:
            break;
        }
    }
}

void InitModule::resetData()
{
    // Reset vars
    m_calibrationInProgress = false;
    m_moduleToCalibrate = -1;
    int skillPos = m_pModuleList[m_usedXtsModuleNumber]->translateSkillIdToSkillPos(POSCALSKILLID);
    m_pModuleList[m_usedXtsModuleNumber]->deregisterTaskForSkill(skillPos);
    m_usedXtsModuleNumber = -1;
    m_waitCondition.wakeAll();
}