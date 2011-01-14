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

private:
    void createActions();
    void createTrayIcon();

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
};

#endif // SWARMTVTRAYICON_H
