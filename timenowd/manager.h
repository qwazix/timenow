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


private:
    QDBusInterface *mceInterface;
    void sleep(int);
    int firstTime;
    bool proximityState;
    bool checkIfLockedAndBlank();
    QDBusMessage modifyProximitydState(QString);
    QString getSetting(QString, QString);
    QString getSetting(QString);


};
#endif // MANAGER_H
