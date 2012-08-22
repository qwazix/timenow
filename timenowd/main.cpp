#include <QtCore/QCoreApplication>
#include <QtDBus>
#include "manager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    manager *myman = new manager();
    qDebug()<<"main";

    return a.exec();
}
