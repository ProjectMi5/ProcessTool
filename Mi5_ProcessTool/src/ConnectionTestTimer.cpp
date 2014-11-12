#include "uaclientsdk.h"
#include <Mi5_ProcessTool/include/ConnectionTestTimer.h>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <iostream>

ConnectionTestTimer::ConnectionTestTimer(IProductionModule* pModule)
{
    m_pModule = pModule;
    m_timer1 = new QTimer(this);
    connect(m_timer1, SIGNAL(timeout()), this, SLOT(timer1update()));
    m_timer1->start(1000);

    m_timer2 = new QTimer(this);
    m_timer2->setSingleShot(true);
    connect(m_timer2, SIGNAL(timeout()), this, SLOT(evaluateConnectionTest()));
}

ConnectionTestTimer::~ConnectionTestTimer()
{
}

void ConnectionTestTimer::timer1update()
{
    m_pModule->writeConnectionTestInput(true);
    m_timer2->start(500);
}

void ConnectionTestTimer::evaluateConnectionTest()
{
    bool result = false;
    result = m_pModule->checkConnectionTestOutput();

    if (result == true)
    {
        // worked
    }

    else
    {
        /*QLOG_DEBUG() << "Module " << m_pModule->getModuleName().toUtf8() << " didnt respond to ConnectionTest"
                  <<
                  ;*/
        m_pModule->moduleDisconnected();
    }

    m_pModule->writeConnectionTestInput(false);

}