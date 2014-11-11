#ifndef TASK_H
#define TASK_H
#include "uaclientsdk.h"
#include <iostream>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>
#include <QMutex>
#include <qobject.h>
#include <qtimer.h>
#include <qthread.h>


class TaskModule;
class Task : public QObject, public ISkillRegistration
{
    Q_OBJECT
public:
    Task(ProductionTask productionTask, std::map<int, IProductionModule*>moduleList,
         TaskModule* taskModule, MessageFeeder* pMessageFeeder);
    ~Task();

public:
    int getTaskId();
    void skillStateChanged(int moduleNumber, int skillPos, int state);
    void start();

public slots:
    void triggerTaskObjectDeletion();

private:
    void evaluateTask();
    void assignSkillsToModules();
    void processNextOpenSkill();
    void evaluateSkillState(int skillNumberInTask);
    matchedSkill assignSingleSkillToModule(taskSkillQueue& nextItem);
    bool isTransportModule(int moduleNumber);

private: //const
    static const int TASKCOUNT = 30;
    static const int SKILLCOUNT = 51;
    static const int PARAMETERCOUNT = 6;

private:
    ProductionTask m_task;
    OpcUa_UInt32 m_state;
    std::map<int, IProductionModule*> m_moduleList;
    TaskModule* m_pTaskModule;
    std::vector<taskSkillQueue> m_skillQueue; /*skillNumberInTask, skillId, state*/
    std::vector<skillModuleList> m_skillListInSystem; /*moduleNumber, skillId, skillPos*/
    std::map<int, matchedSkill> m_matchedSkills;
    MessageFeeder* m_pMsgFeed;
    QMutex mutex;
    QTimer* m_deletionTimer;
    QThread m_thread;
    bool m_foundTransport;
    int m_transportModuleNumber;

private slots:
    void deleteTaskObject();
};

#endif // TASK_H