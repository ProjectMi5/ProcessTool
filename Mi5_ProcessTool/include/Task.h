#ifndef TASK_H
#define TASK_H
#include "uaclientsdk.h"
#include <iostream>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <QThread>
#include <QWaitCondition>

class TaskModule;
//! The Task class is the representation of an ordered Task, waiting to be or being produced.
/*!
    As a new order at the HMI is made, the TaskModule class will construct a new Task object with all the relevant information (e.g. Skills etc.).
*/
class Task : public QObject, public ISkillRegistration
{
    Q_OBJECT
public:
    Task(ProductionTask productionTask, std::map<int, IProductionModule*>moduleList,
         TaskModule* taskModule, MessageFeeder* pMessageFeeder, IProductionModule* pManual);
    ~Task();

public:
    int getTaskId();
    void start();
    void triggerAbortTaskTimeout();

public slots:
    void triggerTaskObjectDeletion();
    void abortTask();
    void skillStateChanged(int moduleNumber, int skillPos, int state);

private:
    void evaluateTask();
    void assignSkillsToModules();
    void evaluateSkillState(int skillNumberInTask);
    matchedSkill assignSingleSkillToModule(taskSkillQueue& nextItem);
    bool isTransportModule(int moduleNumber);

private slots:
    void processNextOpenSkill();

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
    bool m_foundTransport;
    int m_transportModuleNumber;
    IProductionModule* m_pManual;
    bool m_aborted;
    bool m_abortionDone;

private: //Qt
    QMutex m_mutex;
    QTimer* m_deletionTimer;
    QThread m_thread;
    QWaitCondition m_waitCondition;

private slots:
    void deleteTaskObject();
};

#endif // TASK_H