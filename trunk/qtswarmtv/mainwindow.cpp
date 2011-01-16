#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

extern "C" {
#include <swarmtv.h>
}

#include "swarmtvtrayicon.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "singleton.h"
#include "swarmtv.hpp"
#include "swarmtvstats.hpp"
#include "simpletablecontrol.hpp"
#include "sourcetablecontrol.hpp"
#include "readablesize.hpp"
#include "settingsdialog.hpp"
#include "helpdialog.hpp"
#include "searchcontrol.hpp"
#include "downloadedtablecontrol.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Init tray icon
    initTrayIcon();

    // Initialize the table singleton
    simpleTableControl *stc = &Singleton<simpleTableControl>::Instance();
    sourceTableControl *srctc = &Singleton<sourceTableControl>::Instance();
    searchControl *sc = &Singleton<searchControl>::Instance();
    settingsDialog *sd = &Singleton<settingsDialog>::Instance();
    helpDialog *hd = &Singleton<helpDialog>::Instance();
    downloadedTableControl *dtc = &Singleton<downloadedTableControl>::Instance();
    stc->setTable(ui->simpleTableWidget);
    srctc->setTable(ui->sourceTableWidget);
    sc->setUi(ui);

    // Set statistics first time
    this->statsUpdateClicked();

    // Set initial table content
    stc->updateTable();
    srctc->updateTable();

    // Set table size policies
    sc->initHeaders();

    // Initialize Downloaded table
    dtc->setTable(ui->downloadedTableWidget);
    dtc->fillTable();

    // Connect signals
    QObject::connect(ui->statsUpdatePushButton, SIGNAL(clicked()), this, SLOT(statsUpdateClicked()));
    QObject::connect(ui->simpleTableWidget, SIGNAL(cellDoubleClicked(int,int)), stc, SLOT(cellDoubleClicked(int,int)));
    QObject::connect(ui->sourceTableWidget, SIGNAL(cellDoubleClicked(int,int)), srctc, SLOT(cellDoubleClicked(int,int)));
    QObject::connect(ui->sourceAddButton, SIGNAL(clicked()), srctc, SLOT(addButtonClicked()));
    QObject::connect(ui->sourceDelButton, SIGNAL(clicked()), srctc, SLOT(delButtonClicked()));
    QObject::connect(ui->simpleAddButton, SIGNAL(clicked()), stc, SLOT(addSimpleButtonClicked()));
    QObject::connect(ui->simpleDelButton, SIGNAL(clicked()), stc, SLOT(delSimpleButtonClicked()));
    QObject::connect(ui->SimpleEditButton, SIGNAL(clicked()), stc, SLOT(editSimpleButtonClicked()));
    QObject::connect(ui->actionSettings, SIGNAL(triggered()), sd, SLOT(show()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered()), hd, SLOT(show()));
    QObject::connect(ui->searchPushButton, SIGNAL(clicked()), sc, SLOT(searchClicked()));
    QObject::connect(ui->SearchLineEdit, SIGNAL(returnPressed()), sc, SLOT(searchClicked()));
    QObject::connect(ui->searchTableWidget, SIGNAL(cellDoubleClicked(int,int)), sc, SLOT(tableDownload(int,int)));
    QObject::connect(ui->delDownedPushButton, SIGNAL(clicked()), dtc, SLOT(delClicked()));
    QObject::connect(ui->updateDownedPushButton, SIGNAL(clicked()), dtc, SLOT(fillTable()));
    QObject::connect(ui->downloadedTableWidget, SIGNAL(cellDoubleClicked(int,int)), dtc, SLOT(cellDoubleClicked(int,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initTrayIcon()
{
  if(QSystemTrayIcon::isSystemTrayAvailable() == true) {
    // Setup tray icon
    swarmtvTrayIcon* tray = new swarmtvTrayIcon(this);
  }
}

void MainWindow::statsUpdateClicked()
{
    swarmTvStats stats;
    QString 	*statsstr;

    statsstr=stats.getStatsString();
    ui->statsTextEdit->setText(*statsstr);

    delete(statsstr);
}
