#ifndef CONNECTIONTESTTIMER_H
#define CONNECTIONTESTTIMER_H
#include <QTimer>

class ProductionModule;
class ConnectionTestTimer : public QObject
{
    Q_OBJECT

public:
    ConnectionTestTimer(ProductionModule* pModule);
    ~ConnectionTestTimer();

private:
    QTimer* m_timer1;
    QTimer* m_timer2;
    ProductionModule* m_pModule;

private slots:
    void timer1update();
    void evaluateConnectionTest();
};

#endif // CONNECTIONTESTTIMER_H