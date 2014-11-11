#include <Mi5_ProcessTool/include/Task.h>
#include <Mi5_ProcessTool/include/TaskModule.h>
#include <QStringList>

Task::Task(ProductionTask productionTask, std::map<int, IProductionModule*>moduleList,
           TaskModule* taskModule, MessageFeeder* pMessageFeeder) : mutex(QMutex::Recursive),
    m_foundTransport(false), m_state(m_task.taskState), m_transportModuleNumber(-1)
{
    m_task = productionTask;
    m_moduleList = moduleList;
    m_pTaskModule = taskModule;
    m_pMsgFeed = pMessageFeeder;

    moveToThread(&m_thread);
    m_thread.start();
}

Task::~Task()
{
    std::cout << "Task killed";
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
    std::vector<matchedSkill>::iterator chosenModule;

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
                //// We already found a transport, lets use this.5
                for (std::vector<matchedSkill>::iterator it = matchedSkillsVector.begin();
                     it != matchedSkillsVector.end(); it++)
                {
                    if (it->moduleNumber == m_transportModuleNumber)
                    {
                        chosenModule = it;
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
                            chosenModule = it;
                            break;
                        }
                    }
                }
            }
        }
    }
    else if (matchedSkillsVector.size() == 1)
    {
        chosenModule = matchedSkillsVector.begin();
    }
    else
    {
        // no offered skill found. --> Handmodul
        matchedSkill tmpMatchedSkill;
        tmpMatchedSkill.moduleNumber = -1;
        tmpMatchedSkill.taskSkillState = SKILLTASKOPEN;
        tmpMatchedSkill.skillId = -1;
        tmpMatchedSkill.skillPosition = -1;

        std::cout << "Task #" << m_task.taskId << ": Found no suitable module for skill #" <<
                  nextItem.skillNumberInTask << std::endl;

        return tmpMatchedSkill;
    }

    // Write the info to our internal data structure.
    m_task.skill[nextItem.skillNumberInTask].assignedModuleId = chosenModule->moduleNumber;
    m_task.skill[nextItem.skillNumberInTask].assignedModuleName =
        m_moduleList[chosenModule->moduleNumber]->getModuleName();
    m_task.skill[nextItem.skillNumberInTask].assignedModulePosition =
        m_moduleList[chosenModule->moduleNumber]->getModulePosition();
    std::cout << "Task #" << m_task.taskId  << ": Writing assignment for skill position " <<
              chosenModule->skillPosition << " (assignedModule: " << chosenModule->moduleNumber <<
              "), skillNumberInTask" << nextItem.skillNumberInTask <<
              std::endl;

    // Write the info to the outside world and return.
    m_pTaskModule->updateTaskStructure(m_task, nextItem.skillNumberInTask);
    return *chosenModule;
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

            //std::cout << "Deregistered task #" << m_task.taskId << " from module " <<
            //          m_moduleList[m_matchedSkills[skillNumberInTask].moduleNumber] << std::endl;
            std::cout << "Task #" << m_task.taskId << ": Skill #" <<
                      m_matchedSkills[skillNumberInTask].skillPosition << " done." << std::endl;
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
            if (isTransportModule(it->second.moduleNumber))
            {
                // Only recheck, if no transport module is targeted.
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

                        if (paramIdentifier == "XPosition" && ((it->first +
                                                                RelSkillPosOfDependantModule) >= 0)) // sanity check
                        {
                            int moduleNumberOfDependantModule = m_matchedSkills[it->first +
                                                                RelSkillPosOfDependantModule].moduleNumber;
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
        std::cout << "Finished task #" << m_task.taskId << std::endl;

        //triggerTaskObjectDeletion();
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