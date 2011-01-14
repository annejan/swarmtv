#include "swarmtvtrayicon.h"

swarmtvTrayIcon::swarmtvTrayIcon(QMainWindow *parent) :
    QWidget(parent)
{
  // @@TODO Parent should not be NULL
  createActions();
  createTrayIcon();

  trayIcon->setIcon(QIcon(":/swarmtv/mainlogo/resources/swarmtv_logo.png"));

  trayIcon->show();

  QObject::connect(trayIcon, SIGNAL(clicked()), this, SLOT(showHideParent()));
}

void swarmtvTrayIcon::setVisible(bool visible)
{
  QMainWindow *parentwin = static_cast<QMainWindow*>(this->parent());

  if(visible == true) {
    parentwin->show();
  } else {
    parentwin->hide();
  }
}

void swarmtvTrayIcon::createActions()
{
  QMainWindow *parentwin = static_cast<QMainWindow*>(this->parent());

  minimizeAction = new QAction(tr("Mi&nimize"), this);
  connect(minimizeAction, SIGNAL(triggered()), parentwin, SLOT(hide()));

  maximizeAction = new QAction(tr("Ma&ximize"), this);
  connect(maximizeAction, SIGNAL(triggered()), parentwin, SLOT(showMaximized()));

  restoreAction = new QAction(tr("&Restore"), this);
  connect(restoreAction, SIGNAL(triggered()), parentwin, SLOT(showNormal()));

  quitAction = new QAction(tr("&Quit"), this);
  connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}


void swarmtvTrayIcon::createTrayIcon()
{
  trayIconMenu = new QMenu(this);
  trayIconMenu->addAction(minimizeAction);
  trayIconMenu->addAction(maximizeAction);
  trayIconMenu->addAction(restoreAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setContextMenu(trayIconMenu);
}

void swarmtvTrayIcon::showHideParent(){
  QMainWindow *parentwin = static_cast<QMainWindow*>(this->parent());

  if(parentwin->isVisible() == true) {
    parentwin->hide();
  } else {
    parentwin->show();
  }
}
