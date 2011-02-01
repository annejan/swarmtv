#include <QtDBus>
#include <QtGui/QApplication>
#include <QSplashScreen>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
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
