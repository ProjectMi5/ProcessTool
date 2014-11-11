#ifndef INITMODULE_H
#define INITMODULE_H
#include <iostream>
#include "uaclientsdk.h"
#include <Mi5_ProcessTool/include/IModule.h>
#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/ProductionModule.h>
#include <Mi5_ProcessTool/include/GlobalConsts.h>

class OpcuaGateway; // Using forward declaration.

class InitModule : public IModule, ISkillRegistration
{
public:
    InitModule(std::map<int, OpcuaGateway*> pGatewayList,
               std::map<int, IProductionModule*> pModuleList);
    ~InitModule();

public: //IModule methods
    void subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
                                const UaDataNotifications& dataNotifications,
                                const UaDiagnosticInfos&   diagnosticInfos);
    void startup();

public: //ISkillRegistration methods
    int getTaskId();
    void skillStateChanged(int moduleNumber, int skillPos, int state);

public:
    int positionCalibration(int moduleNumber);

private:
    int evalModuleList();
    void positionCalibrationExecution(int moduleNumber, int xtsModuleNumber);
    void resetData();
    void serverReconnected();

private:
    std::map<int, IProductionModule*> m_pModuleList;
    std::map<int, OpcuaGateway*> m_pGatewayList;
    std::vector<int> m_xtsModuleNumbers;
    bool m_calibrationInProgress;
    int m_moduleToCalibrate;
    int m_usedXtsModuleNumber;

};

#endif //INITMODULE_H