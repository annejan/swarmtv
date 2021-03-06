#include <iostream>
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <swarmtv.h>
#include <QMessageBox>

#include "testfilterdialog.hpp"
#include "ui_testfilterdialog.h"
#include "swarmtv.hpp"
#include "singleton.h"
#include "simplecell.hpp"
#include "readablesize.hpp"

testFilterDialog::testFilterDialog(simplefilter_struct &simple, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::testFilterDialog)
{
  ui->setupUi(this);

  // Set delete on close
  setAttribute(Qt::WA_DeleteOnClose);

  // Get the data from SwarmTv and Put it in the table
  handleTest(simple);

  // Set sizing
  initHeaders();

  // Show dialog
  this->show();
}

testFilterDialog::~testFilterDialog()
{
    delete ui;
}

void testFilterDialog::handleTest(simplefilter_struct &simple)
{
  int rc=0;
  newtorrents_container *newtorrents=NULL;

  // Do Filter test
  rc = rsstfindnewtorrents(&simple, &newtorrents, 150, 0);
  if(rc != 0) {
    QMessageBox::warning(this, tr("SwarmTv"),
                         tr("Could not open the sandbox file."),
                         QMessageBox::Ok);
    this->close();
    return;
  }

  // Fill Table
  fillTable(newtorrents);

  // Free container
  rsstfreenewtorrentscontainer(newtorrents);
}

void testFilterDialog::fillTable(newtorrents_container *newtorrents)
{
  int count=0;
  simpleCell *scell=NULL;
  QLabel *cell=NULL;
  QTableWidget *testTable=0;
  readableSize readable;
  std::string humansize;

  // Get table handle
  testTable = ui->testTableWidget;

  // Set number of lines in table
  testTable->setRowCount(newtorrents->nr);
  std::cout << "row number " << newtorrents->nr << std::endl;

  // Set data to table
  while(count < newtorrents->nr){
    scell = new(simpleCell);
    scell->setText(newtorrents->newtorrent[count].title);
    scell->setSimpleId(newtorrents->newtorrent[count].id);
    testTable->setCellWidget(count, 0, scell);
    cell = new(QLabel);
    readable.setSize(newtorrents->newtorrent[count].size);
    readable.getSize(humansize);
    cell->setText(humansize.c_str());
    testTable->setCellWidget(count, 1, cell);
    cell = new(QLabel);
    cell->setText(newtorrents->newtorrent[count].source);
    testTable->setCellWidget(count, 2, cell);
    cell = new(QLabel);
    cell->setText(newtorrents->newtorrent[count].metatype);
    testTable->setCellWidget(count, 3, cell);
    cell = new(QLabel);

    count++;
  }
}

void testFilterDialog::initHeaders()
{
  // Make sure ui is set
  if(ui == NULL){
    std::cerr << "Set ui before calling initHeaders " << __FILE__ << ":" << __LINE__ << std::endl;
  }

  // Set title as filling
#if QT_VERSION >= 0x050000
  ui->testTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
  ui->testTableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
#endif

  // Set Size static
#if QT_VERSION >= 0x050000
  ui->testTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
  ui->testTableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
#endif

  // Set source static
#if QT_VERSION >= 0x050000
  ui->testTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
#else
  ui->testTableWidget->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
#endif

  // Set metatype static
#if QT_VERSION >= 0x050000
  ui->testTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
#else
  ui->testTableWidget->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);
#endif

}
