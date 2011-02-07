#ifndef SWARMTVTRAYICON_H
#define SWARMTVTRAYICON_H

#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QWidget>

class swarmtvTrayIcon : public QWidget
{
    Q_OBJECT
public:
    explicit swarmtvTrayIcon(QMainWindow *parent);

    void setVisible(bool visible);
signals:

public slots:
    void showHideParent();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void createActions();
    void createTrayIcon();

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QMainWindow *parentwin;
};

#endif // SWARMTVTRAYICON_H
