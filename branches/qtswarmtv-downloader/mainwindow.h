#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "swarmtvtrayicon.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void statsUpdateClicked();
    void initTrayIcon();
    void dbusStartReceived(QString);
    void dbusEndReceived(QString);
    void dbusRssReceived(QString);
    void dbusSimpleReceived(QString);
    void dbusSqlReceived(QString);
    void dbusDownedReceived(QString);

private:
    Ui::MainWindow *ui;
    swarmtvTrayIcon *tray;
    void fancyMessage(QString, bool);
};

#endif // MAINWINDOW_H
