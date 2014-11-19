#ifndef CONNECTIONTESTTIMER_H
#define CONNECTIONTESTTIMER_H
#include <QTimer>
#include <QThread>

class IProductionModule;
class ConnectionTestTimer : public QObject
{
    Q_OBJECT

public:
    ConnectionTestTimer(IProductionModule* pModule);
    ~ConnectionTestTimer();

public slots:
    void startUp();

private:
    QTimer* m_timer1;
    QTimer* m_timer2;
    IProductionModule* m_pModule;

private:
    int m_lastConnectionState;

private:
    void connectionStateChanged(int state);
    bool m_connectionTestBool;
    QThread m_thread;
    bool m_disconnected;
    bool m_lastConnectionTestBool;

private slots:
    void timer1update();
    void evaluateConnectionTest();
};

#endif // CONNECTIONTESTTIMER_H