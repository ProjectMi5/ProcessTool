#ifndef CONNECTIONTESTTIMER_H
#define CONNECTIONTESTTIMER_H
#include <QTimer>

class IProductionModule;
class ConnectionTestTimer : public QObject
{
    Q_OBJECT

public:
    ConnectionTestTimer(IProductionModule* pModule);
    ~ConnectionTestTimer();

private:
    QTimer* m_timer1;
    QTimer* m_timer2;
    IProductionModule* m_pModule;

private slots:
    void timer1update();
    void evaluateConnectionTest();
};

#endif // CONNECTIONTESTTIMER_H