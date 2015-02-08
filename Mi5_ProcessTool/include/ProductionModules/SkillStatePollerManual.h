#ifndef SKILLSTATEPOLLERMANUAL_H
#define SKILLSTATEPOLLERMANUAL_H
#include <Mi5_ProcessTool/include/SkillStatePoller.h>

//! The skill state poller provides a periodic check of a manual module's skill state upon construction.
/*!
    After a skill is being executed, an object of this class will be constructed to periodically check the
    skill's state. This has been implemented, because with the use of subscriptions, some state changes were missed.
    This, again, is simlar to the ProductionModule's SkillStatePoller. The only difference is the changed interface structure.
    TODO: Unify!
*/
class SkillStatePollerManual : public SkillStatePoller
{
public:
    SkillStatePollerManual(IProductionModule* productionModule, int moduleNumber, int skillPos,
                           OpcuaGateway* pGateway);
    ~SkillStatePollerManual();

private slots:
    virtual void checkSkillState();

private:
    int m_moduleNumber;
};

#endif //SKILLSTATEPOLLERMANUAL_H