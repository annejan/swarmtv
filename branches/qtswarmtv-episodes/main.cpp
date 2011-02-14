#include <QtDBus>
#include <QtGui/QApplication>
#include <QSplashScreen>
#include <QSettings>
#include "singleapplication.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv, "QtSwarmTv");
    if (a.isRunning()) {
        return 0;
    }

    // Setup the config object
    QCoreApplication::setOrganizationName("ETV");
    QCoreApplication::setOrganizationDomain("swarmtv.nl");
    QCoreApplication::setApplicationName("Swarmtv");

    MainWindow w;

    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/swarmtv/mainlogo/resources/swarmtv_logo.png"));
    splash->show();

    w.show();

    splash->showMessage( "Setting up DBus", Qt::AlignBottom );
    a.processEvents();

    // Connect DBus
    QDBusConnection connection = QDBusConnection::sessionBus();

    splash->finish(&w);
    delete splash;

    return a.exec();
}
