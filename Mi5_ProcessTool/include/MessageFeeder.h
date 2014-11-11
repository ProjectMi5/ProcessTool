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
    MessageFeeder* getPointer();
    void write(UaString string, messageFeedLevel level);

private:
    int m_feedId;
    int m_feedCounter;
    OpcuaGateway* m_pGateway;
    int m_moduleNumber;
};

#endif //MESSAGEFEEDER_H