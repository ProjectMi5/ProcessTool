#include <Mi5_ProcessTool/include/Task.h>
#include <Mi5_ProcessTool/include/TaskModule.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <QStringList>

Task::Task(ProductionTask productionTask, std::map<int, IProductionModule*>moduleList,
           TaskModule* taskModule, MessageFeeder* pMessageFeeder,
           IProductionModule* pManual) : m_foundTransport(false), m_state(m_task.taskState),
    m_transportModuleNumber(-1), m_pManual(pManual),
    m_aborted(false)
{
    m_task = productionTask;
    m_moduleList = moduleList;
    m_pTaskModule = taskModule;
    m_pMsgFeed = pMessageFeeder;

    UaString message = "Received new Task #";
    message += UaString::number(productionTask.taskId);
    m_pMsgFeed->write(message, msgInfo);

    moveToThread(&m_thread);
    QString threadName = "Task" + QString::number(productionTask.taskId);
    m_thread.setObjectName(threadName);
    m_thread.start();
}

Task::~Task()
{
    QLOG_DEBUG() << "Task killed";
    m_thread.quit();
    m_thread.wait();
}

void Task::start()
{
    m_skillListInSystem = m_pTaskModule->getSkillList();
    evaluateTask();
    assignSkillsToModules();
    processNextOpenSkill();
}

int Task::getTaskId()
{
    return m_task.taskId;
}

bool Task::isTransportModule(int moduleNumber)
{
    bool returnVal = false;

    if (moduleNumber >= MODULENUMBERXTSMIN && moduleNumber <= MODULENUMBERXTSMAX)
    {
        returnVal = true;
    }

    return returnVal;
}

void Task::evaluateTask()
{
    m_skillQueue.clear();
    taskSkillQueue tmpQueue;
    int neededSkillCount = 0;

    for (int i = 0; i < SKILLCOUNT; i++)
    {
        if (m_task.skill[i].dummy == false)
        {
            tmpQueue.skillNumberInTask = neededSkillCount;
            tmpQueue.skillId = m_task.skill[i].id;
            tmpQueue.skillState = SKILLTASKOPEN;
            m_skillQueue.push_back(tmpQueue);

            neededSkillCount++;
        }
    }
}

void Task::assignSkillsToModules()
{
    m_matchedSkills.clear();

    for (std::vector<taskSkillQueue>::iterator it = m_skillQueue.begin(); it != m_skillQueue.end();
         it++)
    {
        m_matchedSkills[it->skillNumberInTask] = assignSingleSkillToModule(*it);
    }
}

matchedSkill Task::assignSingleSkillToModule(taskSkillQueue& nextItem)
{
    std::vector<matchedSkill> matchedSkillsVector;
    matchedSkill chosenModule;

    for (std::vector<skillModuleList>::iterator it2 = m_skillListInSystem.begin();
         it2 != m_skillListInSystem.end(); it2++)
    {
        if (nextItem.skillId == it2->skillId)
        {
            matchedSkill tmpMatchedSkill;
            tmpMatchedSkill.moduleNumber = it2->moduleNumber;
            tmpMatchedSkill.taskSkillState = nextItem.skillState;
            tmpMatchedSkill.skillId = nextItem.skillId;
            tmpMatchedSkill.skillPosition = it2->skillPos;

            matchedSkillsVector.push_back(tmpMatchedSkill);
        }
    }

    // Now check the found offered skills.

    if (matchedSkillsVector.size() > 1)
    {
        // Place for some algorithms.
        if (isTransportModule(matchedSkillsVector[0].moduleNumber)) // The first one is sufficient.
        {
            if (m_foundTransport)
            {
                //// We already found a transport, lets use this.
                for (std::vector<matchedSkill>::iterator it = matchedSkillsVector.begin();
                     it != matchedSkillsVector.end(); it++)
                {
                    if (it->moduleNumber == m_transportModuleNumber)
                    {
                        chosenModule = *it;
                        chosenModule.blocked = true;
                        break;
                    }
                }
            }
            else
            {
                while (!m_foundTransport)
                {
                    for (std::vector<matchedSkill>::iterator it = matchedSkillsVector.begin();
                         it != matchedSkillsVector.end(); it++)
                    {
                        if (m_moduleList[it->moduleNumber]->checkSkillReadyState(it->skillId) ==
                            true) // take the first transport module to become ready.
                        {
                            // Lets use this!
                            m_foundTransport = true;
                            m_transportModuleNumber = it->moduleNumber;
                            chosenModule = *it;
                            chosenModule.blocked = true;
                            break;
                        }
                    }
                }
            }
        }
        else if (m_matchedSkills[nextItem.skillNumberInTask].blocked)  // Place for other skills
        {
            // Return the already assigned one.
            chosenModule = m_matchedSkills[nextItem.skillNumberInTask];
        }
        else
        {
            bool searching = true;
            bool tmpStringSent = false;

            while (searching) //TODO: possible endless loop
            {
                for (std::vector<matchedSkill>::iterator it = matchedSkillsVector.begin();
                     it != matchedSkillsVector.end(); it++)
                {
                    if (m_moduleList[it->moduleNumber]->checkSkillReadyState(it->skillId) ==
                        true) // take the first module to become ready.
                    {
                        // Lets use this!
                        chosenModule = *it;
                        searching = false;
                        break;
                    }
                }

                if ((searching == true) && (tmpStringSent == false))
                {
                    UaString tmpMsg;
                    tmpMsg = "Task #";
                    tmpMsg += UaString::number(m_task.taskId);
                    tmpMsg += UaString(": Waiting for Skill ID #");
                    tmpMsg += UaString::number(chosenModule.skillId);
                    tmpMsg += UaString(" to become ready..");
                    QLOG_WARN() << tmpMsg.toUtf8();
                    m_pMsgFeed->write(tmpMsg, msgWarning);
                    tmpStringSent = true;
                }
            }
        }
    }
    else if (matchedSkillsVector.size() == 1)
    {
        chosenModule = *matchedSkillsVector.begin();
    }
    else
    {
        // no offered skill found. --> Handmodul
        matchedSkill tmpMatchedSkill;
        tmpMatchedSkill.moduleNumber = MANUALMODULE1;
        tmpMatchedSkill.taskSkillState = SKILLTASKOPEN;
        tmpMatchedSkill.skillId = MANUALUNIVERSALSKILL;
        tmpMatchedSkill.skillPosition = 0;

        QLOG_WARN() << "Task #" << m_task.taskId << ": Found no suitable module for skill #" <<
                    nextItem.skillNumberInTask ;

        chosenModule = tmpMatchedSkill;
    }

    // Write the info to our internal data structure.
    m_task.skill[nextItem.skillNumberInTask].assignedModuleId = chosenModule.moduleNumber;
    m_task.skill[nextItem.skillNumberInTask].assignedModuleName =
        m_moduleList[chosenModule.moduleNumber]->getModuleName();
    m_task.skill[nextItem.skillNumberInTask].assignedModulePosition =
        m_moduleList[chosenModule.moduleNumber]->getModulePosition();
    QLOG_DEBUG() << "Task #" << m_task.taskId  << ": Writing assignment for skill position " <<
                 chosenModule.skillPosition << " (assignedModule: " << chosenModule.moduleNumber << " - " <<
                 m_moduleList[chosenModule.moduleNumber]->getModuleName().toUtf8() <<
                 "), skillNumberInTask" << nextItem.skillNumberInTask;

    m_task.taskState = TaskAssigned;
    // Write the info to the outside world and return.
    m_pTaskModule->updateTaskStructure(m_task, nextItem.skillNumberInTask);
    return chosenModule;
}


void Task::skillStateChanged(int moduleNumber, int skillPos, int state)
{
    if (!m_aborted && thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "skillStateChanged", Qt::QueuedConnection, Q_ARG(int,
                                  moduleNumber), Q_ARG(int,
                                          skillPos), Q_ARG(int, state));
        return;
    }

    for (std::map<int, matchedSkill>::iterator it = m_matchedSkills.begin();
         it != m_matchedSkills.end(); it++)
    {
        if ((it->second.moduleNumber == moduleNumber) && (it->second.skillPosition == skillPos))
        {
            int skillNumberInTask = it->first;
            m_matchedSkills[skillNumberInTask].moduleSkillState = state;
            m_matchedSkills[skillNumberInTask].moduleSkillReady =
                m_moduleList[moduleNumber]->checkSkillReadyState(
                    it->second.skillId);
            evaluateSkillState(skillNumberInTask);
        }
    }
}

void Task::evaluateSkillState(int skillNumberInTask)
{
    int taskSkillState = m_matchedSkills[skillNumberInTask].taskSkillState;
    int taskSkillStateOfPreviousOne = SKILLTASKFINISHED;

    if (skillNumberInTask > 0)
    {
        taskSkillStateOfPreviousOne = m_matchedSkills[skillNumberInTask - 1].taskSkillState;
    }

    int moduleState = 0;

    if ((taskSkillState == SKILLTASKOPEN) && (taskSkillStateOfPreviousOne == SKILLTASKFINISHED))
    {
        processNextOpenSkill();
    }
    else if (taskSkillState == SKILLTASKINPROCESS)
    {
        switch (m_matchedSkills[skillNumberInTask].moduleSkillState)
        {
        case SKILLMODULEERROR:
            // retry if ready
            moduleState = m_moduleList[m_matchedSkills[skillNumberInTask].moduleNumber]->checkSkillState(
                              m_matchedSkills[skillNumberInTask].skillId);

            //if (SKILLMODULEREADY == moduleState) // TODO: only do this once. possible endless loop
            //{
            //    m_matchedSkills[skillNumberInTask].moduleSkillState = SKILLTASKOPEN;
            //    processNextOpenSkill();
            //    //try again
            //}
            /* else
             {*/
            //// assign skill to new module
            //m_skillListInSystem = m_pTaskModule->getSkillList();
            //m_matchedSkills[skillNumberInTask] = assignSingleSkillToModule(m_skillQueue[skillNumberInTask]);
            //m_matchedSkills[skillNumberInTask].taskSkillState = SKILLTASKOPEN;
            QLOG_ERROR() << "Task #" << m_task.taskId << ": Skill #" <<
                         skillNumberInTask << " ERRORED... Skipping." ;
            m_pTaskModule->updateSkillState(m_task.taskNumberInStructure, skillNumberInTask, SKILLTASKFINISHED);
            m_matchedSkills[skillNumberInTask].taskSkillState = SKILLTASKFINISHED;
            m_moduleList[m_matchedSkills[skillNumberInTask].moduleNumber]->deregisterTaskForSkill(
                m_matchedSkills[skillNumberInTask].skillPosition);

            if (m_aborted)
            {
                m_waitCondition.wakeAll();
            }

            processNextOpenSkill();
            //}

            break;

        case SKILLMODULEDONE:

            //QLOG_DEBUG() << "Deregistered task #" << m_task.taskId << " from module " <<
            //          m_moduleList[m_matchedSkills[skillNumberInTask].moduleNumber] ;
            QLOG_INFO() << "Task #" << m_task.taskId << ": Skill #" <<
                        skillNumberInTask << " done." ;
            m_pTaskModule->updateSkillState(m_task.taskNumberInStructure, skillNumberInTask, SKILLTASKFINISHED);
            m_matchedSkills[skillNumberInTask].taskSkillState = SKILLTASKFINISHED;
            m_moduleList[m_matchedSkills[skillNumberInTask].moduleNumber]->deregisterTaskForSkill(
                m_matchedSkills[skillNumberInTask].skillPosition);

            if (m_aborted)
            {
                m_waitCondition.wakeAll();
            }

            processNextOpenSkill();
            break;

        case SKILLMODULEBUSY:
            break;

        default:
            break;
        }
    }
    else
    {
        // Nothing to do
    }
}

void Task::processNextOpenSkill()
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "processNextOpenSkill", Qt::QueuedConnection);
        return;
    }

    if (m_aborted)
    {
        return;
        // Dont process new skill, when the abortion was requested and/or is in process.
    }

    bool nextSkillToProcess = false;

    for (std::map<int, matchedSkill>::iterator it = m_matchedSkills.begin();
         it != m_matchedSkills.end(); it++)
    {
        if (it->second.taskSkillState == SKILLTASKOPEN)
        {
            if (!(it->second.blocked)) // Only recheck, if no blocked module is targeted.
            {
                it->second = assignSingleSkillToModule(m_skillQueue[it->first]);
            }

            //execute this skill on the correspondent module
            ParameterInputArray tmpParamArray;

            for (int i = 0; i < PARAMETERCOUNT; i++)
            {
                QString paramStringValue(m_task.skill[it->first].parameter[i].stringValue.toUtf8());

                if (paramStringValue.contains("#DP#"))
                {
                    QStringList dynamicParam = paramStringValue.split("#");

                    if (dynamicParam.count() == 4) //e.g. #DP#-2#XPosition
                    {
                        int RelSkillPosOfDependantModule = dynamicParam[2].toInt();
                        QString paramIdentifier = dynamicParam[3];

                        // Check assignment and block the dependent module
                        int skillPosInTaskOfDependantModule = it->first + RelSkillPosOfDependantModule;
                        int moduleNumberOfDependantModule = m_matchedSkills[skillPosInTaskOfDependantModule].moduleNumber;

                        if (RelSkillPosOfDependantModule >=
                            0) // Only reassign, if the dependent module has not been processed yet.
                        {
                            m_matchedSkills[skillPosInTaskOfDependantModule] = assignSingleSkillToModule(
                                        m_skillQueue[skillPosInTaskOfDependantModule]);
                            m_matchedSkills[skillPosInTaskOfDependantModule].blocked = true;
                            // Update the module number, if changed.
                            moduleNumberOfDependantModule = m_matchedSkills[skillPosInTaskOfDependantModule].moduleNumber;
                        }

                        if (paramIdentifier == "XPosition" && ((skillPosInTaskOfDependantModule) >= 0)) // sanity check
                        {
                            tmpParamArray.paramInput[i].value =
                                m_moduleList[moduleNumberOfDependantModule]->getModulePosition();
                            tmpParamArray.paramInput[i].string = "Fischer ftw.";
                        }

                    }
                    else
                    {
                        //something is cookoo
                        tmpParamArray.paramInput[i].value = -1;
                        tmpParamArray.paramInput[i].string = "Wrong or unknown dynamic parameter format.";
                        m_pMsgFeed->write("Wrong or unknown dynamic parameter format.", msgError);
                        QLOG_ERROR() << "Wrong or unknown dynamic parameter format. Parameter[" << i << "] at Skill " <<
                                     it->first;
                    }
                }
                else
                {
                    tmpParamArray.paramInput[i].value = m_task.skill[it->first].parameter[i].value;
                    tmpParamArray.paramInput[i].string = m_task.skill[it->first].parameter[i].stringValue;
                }
            }

            int moduleState = m_moduleList[it->second.moduleNumber]->checkSkillState(it->second.skillId);
            m_moduleList[it->second.moduleNumber]->registerTaskForSkill(this, it->second.skillPosition);

            if (moduleState == SKILLMODULEREADY)
            {
                QLOG_DEBUG() << "Task number in structure #" << m_task.taskNumberInStructure;
                m_moduleList[it->second.moduleNumber]->executeSkill(it->second.skillPosition, tmpParamArray);

                if (it->second.moduleNumber == MANUALMODULE1) //TODO: remove this hack.
                {
                    UaString tmpstring = UaString("Manual action required for Task ID #");
                    tmpstring += UaString::number(m_task.taskId);
                    m_pMsgFeed->write(tmpstring, msgManualActionRequired);
                }

                m_pTaskModule->updateSkillState(m_task.taskNumberInStructure, it->first, SKILLTASKINPROCESS);
                m_matchedSkills[it->first].taskSkillState = SKILLTASKINPROCESS;
            }

            nextSkillToProcess = true;
            break; //we only want the first open skill.
        }
    }

    if (!nextSkillToProcess)
    {
        // finished or error.. go on
        UaString message = "Finished Task ID #";
        message += UaString::number(m_task.taskId);
        m_pMsgFeed->write(message, msgInfo);
        QLOG_INFO() << message.toUtf8();

        //triggerTaskObjectDeletion();
        m_task.taskState = TaskDone;
        m_pTaskModule->notifyTaskDone(m_task.taskId, m_task.taskNumberInStructure, m_task.taskState);
    }
}

void Task::deleteTaskObject()
{
    m_pTaskModule->notifyTaskDone(m_task.taskId, m_task.taskNumberInStructure, m_task.taskState);
}

void Task::triggerTaskObjectDeletion()
{
    //if (thread() != QThread::currentThread())
    //{
    //    QMetaObject::invokeMethod(this, "deleteTaskObject", Qt::QueuedConnection);
    //    return;
    //}

    //m_deletionTimer = new QTimer(this);
    //connect(m_deletionTimer, SIGNAL(timeout()), this, SLOT(deleteTaskObject()));
    //m_deletionTimer->start(3000);
}

void Task::abortTask()
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "abortTask", Qt::QueuedConnection);
        return;
    }

    m_aborted = true;
    m_abortionDone = false;

    for (std::map<int, matchedSkill>::iterator it = m_matchedSkills.begin();
         it != m_matchedSkills.end(); it++)
    {
        if (it->second.taskSkillState == SKILLTASKINPROCESS && !m_abortionDone)
        {
            // Wait til its done or timeout (triggered from outside).
            m_mutex.lock();
            m_waitCondition.wait(&m_mutex);
            m_mutex.unlock();
            m_moduleList[it->second.moduleNumber]->deregisterTaskForSkill(
                m_matchedSkills[it->first].skillPosition);

        }

        it->second.taskSkillState = SKILLTASKERROR;
        m_pTaskModule->updateSkillState(m_task.taskNumberInStructure, it->first, SKILLTASKERROR);
    }

    // Reset XTS
    if (m_foundTransport && !m_abortionDone)
    {
        ParameterInputArray tmpParam;

        for (int i = 0; i < PARAMETERCOUNT; i++)
        {
            tmpParam.paramInput[i].string = UaString("");
            tmpParam.paramInput[i].value = 0;
        }

        std::map<int, matchedSkill>::iterator matchedSkillsIterator;

        if (m_moduleList[m_transportModuleNumber]->isBlocked() && !m_abortionDone)
        {
            int skillPos = m_moduleList[m_transportModuleNumber]->translateSkillIdToSkillPos(SKILLIDXTSUNBLOCK);

            // set skillstateinprocess
            for (matchedSkillsIterator = m_matchedSkills.begin();
                 matchedSkillsIterator != m_matchedSkills.end(); matchedSkillsIterator++)
            {
                if (matchedSkillsIterator->second.skillId == SKILLIDXTSUNBLOCK)
                {
                    matchedSkillsIterator->second.taskSkillState = SKILLTASKINPROCESS;
                }
            }

            m_moduleList[m_transportModuleNumber]->registerTaskForSkill(this, skillPos);
            m_moduleList[m_transportModuleNumber]->executeSkill(skillPos, tmpParam);
            QLOG_DEBUG() << "Unblocked Transport.";

            // Wait til its done.
            m_mutex.lock();
            m_waitCondition.wait(&m_mutex);
            m_mutex.unlock();
        }

        if (m_moduleList[m_transportModuleNumber]->isReserved() && !m_abortionDone)
        {
            int skillPos = m_moduleList[m_transportModuleNumber]->translateSkillIdToSkillPos(SKILLIDXTSRELEASE);

            // set skillstateinprocess
            for (matchedSkillsIterator = m_matchedSkills.begin();
                 matchedSkillsIterator != m_matchedSkills.end(); matchedSkillsIterator++)
            {
                if (matchedSkillsIterator->second.skillId == SKILLIDXTSRELEASE)
                {
                    matchedSkillsIterator->second.taskSkillState == SKILLTASKINPROCESS;
                }
            }

            m_moduleList[m_transportModuleNumber]->registerTaskForSkill(this, skillPos);
            m_moduleList[m_transportModuleNumber]->executeSkill(skillPos, tmpParam);
            QLOG_DEBUG() << "Released Transport.";
            // Wait til its done.
            m_mutex.lock();
            m_waitCondition.wait(&m_mutex);
            m_mutex.unlock();
        }
    }

    m_matchedSkills.clear();
    UaString tmpMsg = UaString("Aborted Task #");
    tmpMsg += UaString::number(m_task.taskId);
    QLOG_INFO() << tmpMsg.toUtf8();
    m_pMsgFeed->write(tmpMsg, msgSuccess);
    m_abortionDone = true;
    m_pTaskModule->notifyTaskDone(m_task.taskId, m_task.taskNumberInStructure, TaskError);
}

void Task::triggerAbortTaskTimeout()
{
    QLOG_DEBUG() << "Timeout: Abort Task";
    m_abortionDone = true;
    m_waitCondition.wakeAll();
}
