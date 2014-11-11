#ifndef MESSAGEFEEDER_H
#define MESSAGEFEEDER_H

#include <Mi5_ProcessTool/include/GlobalConsts.h>
#include "uabase.h"
class OpcuaGateway;

class MessageFeeder
{
public:
    MessageFeeder(OpcuaGateway* pGateway, int moduleNumber);
    ~MessageFeeder();

public:
    void write(UaString string, messageFeedLevel level);

private:
    int m_feedId;
    int m_feedCounter;
    OpcuaGateway* m_pGateway;
    int m_moduleNumber;

private:
    static const int LISTSIZE = 100;

private:
    void writeToOpcua(UaString message, messageFeedLevel level, UaString timestamp);
    void resetList();
};

#endif //MESSAGEFEEDER_H