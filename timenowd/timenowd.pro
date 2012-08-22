#-------------------------------------------------
#
# Project created by QtCreator 2012-07-08T18:19:17
#
#-------------------------------------------------

QT       += core

#QT       -= gui

QT += sql \
      dbus \


TARGET = timenowd
CONFIG   += console
CONFIG   -= app_bundle
#CONFIG += qt mobility
#MOBILITY += sensors

TEMPLATE = app


SOURCES += main.cpp \
    manager.cpp \
    myproximitysensor.cpp

maemo5 {
    target.path = /opt/timenowd/bin
    INSTALLS += target
}

HEADERS += \
    manager.h \
    myproximitysensor.h

OTHER_FILES += \
    shcript \
    timenow.conf

shcriptfile.path = /opt/timenowd/bin
shcriptfile.files = shcript shcriptoff
INSTALLS += shcriptfile

mydaemon.path = /etc/event.d
mydaemon.files = timenow.conf

INSTALLS += mydaemon


