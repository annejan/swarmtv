#include "swarmtvtrayicon.h"

swarmtvTrayIcon::swarmtvTrayIcon(QMainWindow *parent) :
    QWidget(parent)
{
  parentwin = parent;

  createActions();
  createTrayIcon();

  trayIcon->setIcon(QIcon(":/swarmtv/mainlogo/resources/swarmtv_logo.png"));

  trayIcon->show();

  QObject::connect(trayIcon, SIGNAL(triggered()), this, SLOT(showHideParent()));
  QObject::connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void swarmtvTrayIcon::setVisible(bool visible)
{
  if(visible == true) {
    parentwin->show();
  } else {
    parentwin->hide();
  }
}

void swarmtvTrayIcon::createActions()
{
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
//  trayIconMenu = new QMenu(this);
//  trayIconMenu->addAction(minimizeAction);
//  trayIconMenu->addAction(maximizeAction);
//  trayIconMenu->addAction(restoreAction);
//  trayIconMenu->addSeparator();
//  trayIconMenu->addAction(quitAction);

  trayIcon = new QSystemTrayIcon(this);
//  trayIcon->setContextMenu(trayIconMenu);
}

void swarmtvTrayIcon::showHideParent()
{
  if(parentwin->isVisible() == true) {
    parentwin->hide();
  } else {
    parentwin->show();
  }
}

void swarmtvTrayIcon::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
    //    iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1)
    //                                  % iconComboBox->count());
        this->showHideParent();
        break;
    //case QSystemTrayIcon::MiddleClick:
    //    showMessage();
    //    break;
    default:
        ;
    }
}
