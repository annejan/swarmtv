#include <QtDBus>
#include <QtGui/QApplication>
#include <QSplashScreen>
#include "singleapplication.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv, "QtSwarmTv");
    if (a.isRunning()) {
        return 0;
    }

    MainWindow w;

    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/swarmtv/mainlogo/resources/swarmtv_logo.png"));
    splash->show();

    w.show();

    splash->showMessage( "Setting up DBus", Qt::AlignBottom );
    a.processEvents();

    // Connect DBus
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.connect("", "", "nl.swarmtv.dbus", "start", &w, SLOT(dbusStartReceived(QString)));
    connection.connect("", "", "nl.swarmtv.dbus", "end", &w, SLOT(dbusEndReceived(QString)));
    connection.connect("", "", "nl.swarmtv.dbus", "rss", &w, SLOT(dbusRssReceived(QString)));
    connection.connect("", "", "nl.swarmtv.dbus", "simple", &w, SLOT(dbusSimpleReceived(QString)));
    connection.connect("", "", "nl.swarmtv.dbus", "sql", &w, SLOT(dbusSqlReceived(QString)));
    connection.connect("", "", "nl.swarmtv.dbus", "downed", &w, SLOT(dbusDownedReceived(QString)));

    splash->finish(&w);
    delete splash;

    return a.exec();
}
