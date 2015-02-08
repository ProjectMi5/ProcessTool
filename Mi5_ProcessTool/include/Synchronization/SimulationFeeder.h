#ifndef SIMULATIONFEEDER_H
#define SIMULATIONFEEDER_H
#include "uaclientsdk.h"
#include <QObject>
#include <QTimer>
#include <QThread>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/MessageFeeder.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>

class OpcuaGateway;
//! The SimulationFeeder class provides an interface to exchange data with the Simulation/Visualization.
/*!
    This class periodically writes state information to an OPC UA server, where Industrial Physics grabs this data.
*/
class SimulationFeeder : public QObject
{
    Q_OBJECT
public:
    SimulationFeeder(OpcuaGateway* pOpcuaGateway, int moduleNumber,
                     std::map<int, IProductionModule*> moduleList, MessageFeeder* pMsgFeeder);
    ~SimulationFeeder();

private:
    void writePositionInfo();
    void getPositions();

private slots:
    void cyclicAction();

private:
    OpcuaGateway* m_pGateway;
    int m_moduleNumber;
    MessageFeeder* m_pMsgFeeder;
    std::map<int, IProductionModule*> m_moduleList;
    QTimer* m_timer;
    QThread m_thread;
    SimulationData data;

};

#endif //SIMULATIONFEEDER_H