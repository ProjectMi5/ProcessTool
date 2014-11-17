#ifndef IPRODUCTIONMODULE_H
#define IPRODUCTIONMODULE_H

#include <Mi5_ProcessTool/include/IModule.h>

class ISkillRegistration;
class IProductionModule : public IModule
{

public:
    virtual std::map<int, int> getSkills() = 0;
    virtual int checkSkillState(int& skillId) = 0;
    virtual bool checkSkillReadyState(int& skillId) = 0;
    virtual int translateSkillIdToSkillPos(int skillId) = 0;
    virtual int translateSkillPosToSkillId(int skillPos) = 0;
    virtual void assignSkill(int& taskId, Skill skill, int& skillPos) = 0;
    virtual void executeSkill(int& skillPos, ParameterInputArray& paramInput) = 0;
    virtual void deregisterTaskForSkill(int& skillPos) = 0;
    virtual UaString getSkillName(int& skillPos) = 0;
    virtual UaString getModuleName() = 0;
    virtual int getModulePosition() = 0;
    virtual int registerTaskForSkill(ISkillRegistration* pTask, int skillPos) = 0;
    virtual void writeConnectionTestInput(bool input) = 0;
    virtual int checkConnectionTestOutput() = 0;
    virtual void moduleConnectionStatusChanged(int state) = 0;
    virtual bool isBlocked() = 0;
    virtual bool isReserved() = 0;
    virtual void changeModuleMode(int mode) = 0;
};

enum moduleSkillState
{
    SKILLMODULEBUSY = 0,
    SKILLMODULEDONE,
    SKILLMODULEERROR,
    SKILLMODULEREADY
};

enum skillState
{
    SKILLTASKOPEN = 0,
    SKILLTASKINPROCESS,
    SKILLTASKFINISHED,
    SKILLTASKERROR
};

#endif // IPRODUCTIONMODULE_H