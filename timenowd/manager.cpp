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
                       SLOT(controlPolling(const QDBusMessage&)));
    mceInterface = new QDBusInterface(MCE_SERVICE, MCE_REQUEST_PATH,
                                      MCE_REQUEST_IF, connection, this);

    QDBusMessage msg = QDBusMessage::createMethodCall(
            "proximityd.method.change", // --dest
            "/proximityd/method/change", // destination object path
            "proximityd.method.change", // message name (w/o method)
            "Change" // method
        );
     msg << QString("turnOn");  // number hidden in posting
     msg << getpid();
     QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
     qDebug()<<reply;

    QDBusConnection sessionConnection = QDBusConnection::sessionBus();
    sessionConnection.connect("", "/proximityd/signal/state", "proximityd.signal.state", "changed", this, SLOT(printTime(QString)));

    timerOff = new QTimer(this);
    connect(timerOff, SIGNAL(timeout()), this, SLOT(off()));
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(refreshTime()));

    //QDBusInterface *Interface = new QDBusInterface(QString("com.nokia.mce"), QString("/com/nokia/mce/request"), QString("com.nokia.mce.request.get_tklock_mode"), bus);
    //Interface->connection().connect("com.nokia.mce", "/com/nokia/mce/request", "com.nokia.mce.request.get_tklock_mode", "status_changed", this, SLOT(printTime(QString(" "))));
    //QDBusConnection::systemBus().connect("com.nokia.mce", "/com/nokia/mce/request", "com.nokia.mce.request.get_tklock_mode", "status_changed", this, SLOT(printTime(QString(" "))));
    //printTime("d");
}

manager::~manager(){
    QDBusMessage msg = QDBusMessage::createMethodCall(
            "proximityd.method.change", // --dest
            "/proximityd/method/change", // destination object path
            "proximityd.method.change", // message name (w/o method)
            "Change" // method
        );
     msg << QString("turnOff");  // number hidden in posting
     msg << getpid();
     QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
     qDebug()<<reply;
}

void manager::controlPolling(QDBusMessage& reply){
    QString status = reply.arguments().value(0).value<QString>();
    if (status == MCE_TK_LOCKED) timerCheck->start(); else timerCheck->stop();
}

void manager::printTime(QString state){
    qDebug("stateChanged");
    if (checkIfLockedAndBlank()&&state=="open"){
          QProcess::startDetached( "/opt/timenowd/bin/shcript", QStringList() << " " );
          sleep(500);
          QDateTime now = QDateTime::currentDateTime();
          QFile file("/sys/class/graphics/fb0/rotate");
          int x = 35, s = 14, sdate = 3, xdate = -300;
          if ( file.open(QIODevice::ReadOnly) ) {
          // file opened successfully
                  QTextStream t( &file );        // use a text stream
                  if ( t.readLine().compare("3", Qt::CaseSensitive) == 0 ) {
                      x = 0;
                      s = 10;
                      sdate = 2;
                      xdate = -100;
                  }
          }
            QProcess::startDetached( "/usr/bin/text2screen", QStringList() << "-t " + now.toString("HH:mm") << "-s "+ QString::number(s) << "-T 0xFFFFFF" << "-B 0x000000" << "-H center" << "-y 0" );
            QProcess::startDetached( "/usr/bin/text2screen", QStringList() << "-t " + now.toString("dddd dd/MM/yyyy") << "-s "+ QString::number(sdate) << "-T 0xFFFFFF" << "-B 0x000000" << "-x "+QString::number(xdate) << "-y 300" );
//          printApixel();
//          timer->start(60000);
          timerOff->start(6000);

    } else {
        if (timer->isActive()) timer->stop();
        //if (firstTime==0&&timer->isActive()) timer->stop(); else firstTime = 0;
    }
}

void manager::off(){
    if (checkIfLockedAndBlank()){
        QProcess::startDetached( "/opt/timenowd/bin/shcriptoff", QStringList() << " " );
    }
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


//get notifications
//create control panel applet
//dbus to get notified for configuration changes
//image?
//better fonts?
//date

void manager::refreshTime(){
    QDateTime now = QDateTime::currentDateTime();
    QProcess::startDetached( "/usr/bin/text2screen", QStringList() << "-t " + now.toString("HH:mm") << "-s 14 " << "-T 0xFFFFFF" << "-B 0x000000" << "-x 35" << "-y 0" );
}


void manager::printApixel(){

    int fbfd = 0;
            struct fb_var_screeninfo vinfo;
            struct fb_fix_screeninfo finfo;
            long int screensize = 0;
             char *fbp = 0;
            int x = 0, y = 0;
            long int location = 0;
            int count ;

            /* Open the file for reading and writing */
            fbfd = open("/dev/fb0", O_RDWR);
            if (!fbfd) {
                    printf("Error: cannot open framebuffer device.\n");
                    exit(1);
            }
            printf("The framebuffer device was opened successfully.\n");
         /* Get fixed screen information */
            if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
                   printf("Error reading fixed information.\n");
                    exit(2);
            }

            /* Get variable screen information */
            if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
                    printf("Error reading variable information.\n");
                    exit(3);
            }

            /* Figure out the size of the screen in bytes */
            screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
            printf("\nScreen size is %d",screensize);
            printf("\nVinfo.bpp = %d",vinfo.bits_per_pixel);

            /* Map the device to memory */
            fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,fbfd, 0);
            if ((int)fbp == -1) {
                    printf("Error: failed to map framebuffer device to memory.\n");
                    exit(4);
            }
             printf("The framebuffer device was mapped to memory successfully.\n");


            x = 100; y = 100; /* Where we are going to put the pixel */

            /* Figure out where in memory to put the pixel */
            location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
            for(count = 1 ;count < 200 ;count++)
            {
                    *(fbp + location) = 255;    /* Some blue */
                    *(fbp + location + count) = 100; /* A little green */
                    *(fbp + location + count + 1) = 100; /* A lot of red */
                    *(fbp + location + count + 2) = 100; /* No transparency */
            }
            munmap(fbp, screensize);
            close(fbfd);



}

bool manager::checkIfLockedAndBlank(){
    const QDBusMessage& reply = mceInterface->call(MCE_DISPLAY_STATUS_GET);
    QString display = reply.arguments().value(0).value<QString>();

    const QDBusMessage& reply2 = mceInterface->call(MCE_TKLOCK_MODE_GET);
    QString locked = reply2.arguments().value(0).value<QString>();

    if (display == MCE_DISPLAY_OFF_STRING && locked == MCE_TK_LOCKED){
        return true;
    } else return false;
}


void manager::sleep(int ms)
{
    if (ms > 0){
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
    }
}
