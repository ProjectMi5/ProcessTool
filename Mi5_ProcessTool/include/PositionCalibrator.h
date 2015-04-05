#ifndef INITMODULE_H
#define INITMODULE_H
#include <iostream>

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include "uaclientsdk.h"

#include <Mi5_ProcessTool/include/ISkillRegistration.h>
#include <Mi5_ProcessTool/include/ProductionModule.h>
#include <Mi5_ProcessTool/include/GlobalConsts.h>
#include <Mi5_ProcessTool/include/IModule.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>

class OpcuaGateway; // Using forward declaration.

//! The PositionCalibrator class provides an interface to the XTS for position calibration.
/*!
   ..
*/
class PositionCalibrator : public QObject, IModule, ISkillRegistration
{
    Q_OBJECT
public:
    PositionCalibrator(std::map<int, OpcuaGateway*> pGatewayList,
                       std::map<int, IProductionModule*> pModuleList, MessageFeeder* pMsgFeeder);
    ~PositionCalibrator();

public: //IModule methods
    void subscriptionDataChange(OpcUa_UInt32               clientSubscriptionHandle,
                                const UaDataNotifications& dataNotifications,
                                const UaDiagnosticInfos&   diagnosticInfos);
    void startup();
    void serverReconnected();

public: //ISkillRegistration methods
    int getTaskId();
    void skillStateChanged(int moduleNumber, int skillPos, int state);

public:
    int positionCalibration(int moduleNumber);

private:
    int evalModuleList();
    void positionCalibrationExecution(int moduleNumber, int xtsModuleNumber);
    void resetData();

private:
    std::map<int, IProductionModule*> m_pModuleList;
    std::map<int, OpcuaGateway*> m_pGatewayList;
    std::vector<int> m_xtsModuleNumbers;
    bool m_calibrationInProgress;
    int m_moduleToCalibrate;
    int m_usedXtsModuleNumber;
    QThread m_thread;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    MessageFeeder* m_pMsgFeeder;
    int m_oldState;
};

#endif //INITMODULE_H