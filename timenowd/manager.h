#include "QObject"
#include "QDBusConnection"
#ifndef MANAGER_H
#define MANAGER_H
#include <QTimer>
#include <QSettings>

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
    void checkProximity();
    void off();
    void controlPolling(QDBusMessage&);
    void controlPolling(QString);
    void killEverybody();
    void pressPower();

private:
    QDBusInterface *mceInterface;
    void sleep(int);
    int firstTime;
    bool proximityState;
    QTimer *keepTkLockOn, *pressPowerTimer;
    void setLockScreenMode(QString);
    void setDisplayMode(QString);
    bool checkIfLockedAndBlank();
    bool checkIfLocked();
    QDBusMessage modifyProximitydState(QString);
    QString getSetting(QString, QString);
    QString getSetting(QString);



};
#endif // MANAGER_H
