#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
};

#endif // MAINWINDOW_H
