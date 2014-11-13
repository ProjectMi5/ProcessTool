#include <Mi5_ProcessTool/include/Task.h>
#include <Mi5_ProcessTool/include/TaskModule.h>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <QStringList>

Task::Task(ProductionTask productionTask, std::map<int, IProductionModule*>moduleList,
           TaskModule* taskModule, MessageFeeder* pMessageFeeder,
           IProductionModule* pManual) : mutex(QMutex::Recursive),
    m_foundTransport(false), m_state(m_task.taskState), m_transportModuleNumber(-1), m_pManual(pManual)
{
    m_task = productionTask;
    m_moduleList = moduleList;
    m_pTaskModule = taskModule;
    m_pMsgFeed = pMessageFeeder;

    UaString message = "Received new Task #";
    message += UaString::number(productionTask.taskId);
    m_pMsgFeed->write(message, msgInfo);

    moveToThread(&m_thread);
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

            while (searching)
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
                 chosenModule.skillPosition << " (assignedModule: " << chosenModule.moduleNumber <<
                 "), skillNumberInTask" << nextItem.skillNumberInTask;

    m_task.taskState = TaskAssigned;
    // Write the info to the outside world and return.
    m_pTaskModule->updateTaskStructure(m_task, nextItem.skillNumberInTask);
    return chosenModule;
}



void Task::skillStateChanged(int moduleNumber, int skillPos, int state)
{

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

            if (SKILLMODULEREADY == moduleState) // TODO: only do this once. possible endless loop
            {
                m_matchedSkills[skillNumberInTask].moduleSkillState = SKILLTASKOPEN;
                processNextOpenSkill();
                //try again
            }
            else
            {
                // assign skill to new module
            }

            break;

        case SKILLMODULEDONE:

            //QLOG_DEBUG() << "Deregistered task #" << m_task.taskId << " from module " <<
            //          m_moduleList[m_matchedSkills[skillNumberInTask].moduleNumber] ;
            QLOG_INFO() << "Task #" << m_task.taskId << ": Skill #" <<
                        skillNumberInTask << " done." ;
            m_matchedSkills[skillNumberInTask].taskSkillState = SKILLTASKFINISHED;
            m_moduleList[m_matchedSkills[skillNumberInTask].moduleNumber]->deregisterTaskForSkill(
                m_matchedSkills[skillNumberInTask].skillPosition);
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
                                     it->first
                                     ;
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

                m_matchedSkills[it->first].taskSkillState = SKILLTASKINPROCESS;
            }

            nextSkillToProcess = true;
            break; //we only want the first open skill.
        }
    }

    if (!nextSkillToProcess)
    {
        // finished or error.. go on
        QLOG_INFO() << "Finished task ID #" << m_task.taskId;
        UaString message = "Finished Task #";
        message += UaString::number(m_task.taskId);
        m_pMsgFeed->write(message, msgInfo);

        //triggerTaskObjectDeletion();
        m_task.taskState = TaskDone;
        m_pTaskModule->notifyTaskDone(m_task.taskId, m_task.taskNumberInStructure);

    }
}

void Task::deleteTaskObject()
{
    m_pTaskModule->notifyTaskDone(m_task.taskId, m_task.taskNumberInStructure);
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
    for (std::map<int, matchedSkill>::iterator it = m_matchedSkills.begin();
         it != m_matchedSkills.end(); it++)
    {
        if (it->second.taskSkillState == SKILLTASKINPROCESS)
        {
            m_moduleList[it->second.moduleNumber]->deregisterTaskForSkill(
                m_matchedSkills[it->second.moduleNumber].skillPosition);
        }

        it->second.taskSkillState = SKILLTASKERROR;
    }

    m_matchedSkills.clear();
    m_pTaskModule->notifyTaskDone(m_task.taskId, m_task.taskNumberInStructure);
    QLOG_INFO() << "Aborted Task #" << m_task.taskId;
}
