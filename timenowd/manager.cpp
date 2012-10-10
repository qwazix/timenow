#include "manager.h"
//#include "myproximitysensor.h"
#include "QDebug"
#include <QDBusInterface>
#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <QProcess>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <QFile>
#include <QSettings>
#include <QDir>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define PROXIMITY_STATE "/sys/devices/platform/gpio-switch/proximity/state"

manager::manager()
{
    //firstTime = 1;
    QDBusConnection connection = QDBusConnection ::systemBus();
    connection.connect(MCE_SERVICE, MCE_SIGNAL_PATH, MCE_SIGNAL_IF,
                       MCE_TKLOCK_MODE_SIG, this,
                       SLOT(controlPolling(QString)));

    mceInterface = new QDBusInterface(MCE_SERVICE, MCE_REQUEST_PATH,
                                      MCE_REQUEST_IF, connection, this);

    QDBusMessage msg = modifyProximitydState("turnOn");
    msg << QString("changeInterval");
    msg << getpid();
    msg << getSetting("interval","600").toInt();
    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);

    QDBusConnection sessionConnection = QDBusConnection::sessionBus();
    sessionConnection.connect("", "/proximityd/signal/state", "proximityd.signal.state", "changed", this, SLOT(printTime(QString)));

    keepTkLockOn = new QTimer(this);
    keepTkLockOn->setSingleShot(true);
    keepTkLockOn->setInterval(getSetting("timeout","8000").toInt());
    pressPowerTimer = new QTimer(this);
    pressPowerTimer->setInterval(4100);
    connect(keepTkLockOn, SIGNAL(timeout()), this, SLOT(off()));
    connect(pressPowerTimer, SIGNAL(timeout()), this, SLOT(pressPower()));
//    timerOff = new QTimer(this);
//    connect(timerOff, SIGNAL(timeout()), this, SLOT(off()));
//    timer = new QTimer(this);
//    connect(timer, SIGNAL(timeout()), this, SLOT(refreshTime()));

    //QDBusInterface *Interface = new QDBusInterface(QString("com.nokia.mce"), QString("/com/nokia/mce/request"), QString("com.nokia.mce.request.get_tklock_mode"), connection);
    //Interface->connection().connect("com.nokia.mce", "/com/nokia/mce/request", "com.nokia.mce.request.get_tklock_mode", "status_changed", this, SLOT(killEverybody()));
    //QDBusConnection::systemBus().connect("com.nokia.mce", "/com/nokia/mce/signal", "com.nokia.mce.signal", "tklock_mode_ind", this, SLOT(killEverybody()));
    //printTime("d");
}

manager::~manager(){
     modifyProximitydState("turnOff");
}

QDBusMessage manager::modifyProximitydState(QString newState){
    QDBusMessage msg = QDBusMessage::createMethodCall(
            "proximityd.method.change", // --dest
            "/proximityd/method/change", // destination object path
            "proximityd.method.change", // message name (w/o method)
            "Change" // method
        );
    if (newState != ""){
         msg << newState;
         msg << getpid();
         QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
//         qDebug()<<reply;
    }
    return msg;
}

void manager::controlPolling(QDBusMessage& reply){
//    qDebug("checking");
    QString status = reply.arguments().value(0).value<QString>();
    controlPolling(status);
}

void manager::controlPolling(QString status){
    if (status == MCE_TK_LOCKED) {
        modifyProximitydState("turnOn");
    } else {
        modifyProximitydState("turnOff");
        pressPowerTimer->stop();
//        killEverybody();
    }
}

void manager::setLockScreenMode(QString mode){
    QDBusMessage msg = QDBusMessage::createMethodCall(
            "com.nokia.mce", // --dest
            "/com/nokia/mce/request", // destination object path
            "com.nokia.mce.request", // message name (w/o method)
            "req_tklock_mode_change" // method
        );
         msg << mode;
         QDBusMessage reply = QDBusConnection::systemBus().call(msg);
//         qDebug()<<reply;

}

void manager::setDisplayMode(QString mode){
    QDBusMessage msg = QDBusMessage::createMethodCall(
            "com.nokia.mce", // --dest
            "/com/nokia/mce/request", // destination object path
            "com.nokia.mce.request", // message name (w/o method)
            "req_display_state_" + mode // method
        );
         msg << mode;
         QDBusMessage reply = QDBusConnection::systemBus().call(msg);
}

void manager::pressPower(){
    if (checkIfLocked()) QProcess::startDetached( "/opt/timenowd/bin/shcript", QStringList() << " " );
}

void manager::printTime(QString state){
//    if (!checkIfLockedAndBlank())
    if (state=="open"){
        if (checkIfLockedAndBlank()){
            qDebug()<<"let's start dancing";
            pressPowerTimer->start();
            keepTkLockOn->start();
            pressPower();
        }
    } else {
        qDebug()<<"stop and lock";
        pressPowerTimer->stop();
        keepTkLockOn->stop();
        if (checkIfLocked()) setDisplayMode("off");
        setLockScreenMode("locked");
    }

}

void manager::off(){
    pressPowerTimer->stop();
    qDebug()<<"enough";
    if (checkIfLockedAndBlank()){
        QProcess::startDetached( "/opt/timenowd/bin/shcriptoff", QStringList() << " " );
    }
}

void manager::killEverybody(){
    qDebug()<<"killing all";
    QProcess::startDetached( "killall", QStringList() << "shcript" );
}

void manager::checkProximity()
{
      /* cat /sys/devices/platform/gpio-switch/proximity/state */
      QFile file(PROXIMITY_STATE);
      if ( file.open(QIODevice::ReadOnly) ) {
      // file opened successfully
              QTextStream t( &file );        // use a text stream
              if ( t.readLine().compare("open", Qt::CaseSensitive) == 0 ) {
                  if (proximityState == false) {
                      proximityState = true;
                      emit sensorOpen();
                  }
              } else {
                  proximityState = false;
              }
      file.close();
  }
}


bool manager::checkIfLockedAndBlank(){
    const QDBusMessage& reply = mceInterface->call(MCE_DISPLAY_STATUS_GET);
    QString display = reply.arguments().value(0).value<QString>();

    const QDBusMessage& reply2 = mceInterface->call(MCE_CALL_STATE_GET);
    QString callstatus = reply2.arguments().value(0).value<QString>();

    if (display == MCE_DISPLAY_OFF_STRING && checkIfLocked() && callstatus != MCE_CALL_STATE_ACTIVE && callstatus != MCE_CALL_STATE_RINGING){
        return true;
    } else return false;
}

bool manager::checkIfLocked(){
    const QDBusMessage& reply2 = mceInterface->call(MCE_TKLOCK_MODE_GET);
    QString locked = reply2.arguments().value(0).value<QString>();
    qDebug()<<locked;
    return (locked == MCE_TK_LOCKED);
}


void manager::sleep(int ms)
{
    if (ms > 0){
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
    }
}

QString manager::getSetting(QString name, QString defaultval){
    QString settingsFilePath = QDir::homePath () + "/.timenow/settings.ini";
    if (!QFile::exists(settingsFilePath)) {
        if (!QDir(QDir::homePath () + "/.timenow").exists()) QDir(QDir::homePath ()).mkdir(".timenow");
        QFile::copy("/opt/timenowd/conf/settings.ini", settingsFilePath);
    }
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.beginGroup("display");
    return settings.value(name, defaultval).toString();
    settings.endGroup();
}

QString manager::getSetting(QString name){
    return getSetting(name, "");
}
