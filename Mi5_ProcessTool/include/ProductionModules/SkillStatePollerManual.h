#ifndef SKILLSTATEPOLLERMANUAL_H
#define SKILLSTATEPOLLERMANUAL_H
#include <Mi5_ProcessTool/include/SkillStatePoller.h>

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