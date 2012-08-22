#include "QObject"
#include "QDBusConnection"
#ifndef MANAGER_H
#define MANAGER_H
#include <QTimer>

class QDBusMessage;
class QDBusInterface;

class manager : public QObject
{
    Q_OBJECT

public:
    manager();
    ~manager();

signals:
    void sensorOpen();

public Q_SLOTS:
    void printTime(QString);
    void refreshTime();
    void checkProximity();
    void off();
    void controlPolling(QDBusMessage&);


private:
    QDBusInterface *mceInterface;
    QTimer *timer;
    QTimer *timerCheck;
    QTimer *timerOff;
    void printApixel();
    void sleep(int);
    int firstTime;
    bool proximityState;
    bool checkIfLockedAndBlank();


};
#endif // MANAGER_H
