#ifndef ISKILLREGISTRATION_H
#define ISKILLREGISTRATION_H

class ISkillRegistration
{
public:
    virtual int getTaskId() = 0;
    virtual void skillStateChanged(int moduleNumber, int skillPos, int state) = 0;

};

#endif //ISKILLREGISTRATION_H