#ifndef INITMANAGER_H
#define INITMANAGER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <Mi5_ProcessTool/include/QsLog/QsLog.h>
#include <Mi5_ProcessTool/include/PositionCalibrator.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>

class OpcuaGateway;
//! The InitManager class provides a way to synchronize the initialization tasks.
/*!
    A newly constructed module will register itself with the InitManager to demand an initialization.
    So far, the only initialization method available is the position calibration. This class provides a way
    to calibrate registered modules sequentially.
*/
class InitManager : public QObject
{
    Q_OBJECT
public:
    InitManager(bool initialInit);
    ~InitManager();

public:
    int enqueueForInit(int moduleNumber);
    void setConnections(std::map<int, OpcuaGateway*> pGatewayList,
                        std::map<int, IProductionModule*> pModuleList, MessageFeeder* pMsgFeeder);
    bool isInitialInitDone();

public slots:
    int startUpSystem();

private:
    PositionCalibrator* m_positionCalibrator;
    std::map<int, OpcuaGateway*> m_pGatewayList;
    std::map<int, IProductionModule*> m_pModuleList;
    MessageFeeder* m_pMsgFeeder;
    std::vector<int> m_initQueue; /* moduleNumber */
    bool m_initialInitDone;

private: //Qt
    QTimer* m_timer;
    QThread m_thread;
    bool m_initialInit;

private slots:
    void evalInitDemands();

};

#endif //INITMANAGER_H