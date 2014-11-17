#include "uaclientsdk.h"
#include <Mi5_ProcessTool/include/ConnectionTestTimer.h>
#include <Mi5_ProcessTool/include/DataStructures.h>
#include <Mi5_ProcessTool/include/IProductionModule.h>
#include <iostream>

ConnectionTestTimer::ConnectionTestTimer(IProductionModule* pModule) : m_lastConnectionState(-1),
    m_connectionTestBool(false)
{
    m_pModule = pModule;
    m_timer1 = new QTimer(this);
    connect(m_timer1, SIGNAL(timeout()), this, SLOT(timer1update()));

    m_timer2 = new QTimer(this);
    m_timer2->setSingleShot(true);
    connect(m_timer2, SIGNAL(timeout()), this, SLOT(evaluateConnectionTest()));

    moveToThread(&m_thread);
    m_thread.start();
}

ConnectionTestTimer::~ConnectionTestTimer()
{
}

void ConnectionTestTimer::timer1update()
{
    m_connectionTestBool = !m_connectionTestBool;
    m_pModule->writeConnectionTestInput(m_connectionTestBool);
    m_timer2->start(500);
}

void ConnectionTestTimer::evaluateConnectionTest()
{
    int result = !m_connectionTestBool; // Make sure its different.
    result = m_pModule->checkConnectionTestOutput();

    if (result == m_connectionTestBool)
    {
        // worked
        connectionStateChanged(ModuleConnectionConnected);
    }
    else // Wrong or -1
    {
        connectionStateChanged(ModuleConnectionDisconnected);
    }
}

void ConnectionTestTimer::connectionStateChanged(int state)
{
    if (state != m_lastConnectionState)
    {
        m_pModule->moduleConnectionStatusChanged(state);
        m_lastConnectionState = state;
    }
}

void ConnectionTestTimer::startUp()
{
    if (thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "startUp", Qt::QueuedConnection);
        return;
    }

    m_timer1->start(1500);
}
