#include <QMessageBox>
#include <QTableWidget>
#include <iostream>

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

#define PAGE_LIMIT 500

extern "C" {
#include <swarmtv.h>
}

#include "searchcontrol.hpp"
#include "simplecell.hpp"
#include "singleton.h"
#include "swarmtv.hpp"
#include "readablesize.hpp"
#include "newtorrentfullinfodialog.hpp"

searchControl::searchControl(QWidget *parent) :
    QWidget(parent)
{
  this->ui = ui;


  // Connect signals
}

void searchControl::setUi(Ui::MainWindow *ui)
{
  this->ui = ui;

  // Add context menu
  ui->searchTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->searchTableWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(showContextMenu(const QPoint&)));

}

void searchControl::fillTable(newtorrents_container *newtorrents)
{
  int count=0;
  simpleCell *scell=NULL;
  QLabel *cell=NULL;
  QTableWidget *searchTable=0;
  readableSize readable;
  std::string humansize;

  // Get table handle
  searchTable = ui->searchTableWidget;

  // Set number of lines in table
  searchTable->setRowCount(newtorrents->nr);

  // Set data to table
  while(count < newtorrents->nr){
    scell = new(simpleCell);
    scell->setText(newtorrents->newtorrent[count].title);
    scell->setSimpleId(newtorrents->newtorrent[count].id);
    ui->searchTableWidget->setCellWidget(count, 0, scell);
    cell = new(QLabel);
    readable.setSize(newtorrents->newtorrent[count].size);
    readable.getSize(humansize);
    cell->setText(humansize.c_str());
    ui->searchTableWidget->setCellWidget(count, 1, cell);
    cell = new(QLabel);
    cell->setText(newtorrents->newtorrent[count].source);
    ui->searchTableWidget->setCellWidget(count, 2, cell);
    cell = new(QLabel);
    cell->setText(newtorrents->newtorrent[count].metatype);
    ui->searchTableWidget->setCellWidget(count, 3, cell);
    cell = new(QLabel);

    count++;
  }
}

void	searchControl::searchClicked()
{
  int rc=0;
  //simplefilter_struct filter;
  newtorrents_container *newtorrents=NULL;
  char *searchValue=NULL;

  // Get SwarmTv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Get value from
  searchValue =strdup(ui->SearchLineEdit->text().toUtf8().data());

  // Execute query
  rc = rsstfindnewtorrentsbytitle(swarm->getHandle(), searchValue, &newtorrents, PAGE_LIMIT, 0);
  if(rc != 0) {
    return;
  }

  // @DEBUG
  std::cout << "Search started: '" << newtorrents->nr << "' query: '" << searchValue << "'" <<  std::endl;

  // Fill Table with search results
  fillTable(newtorrents);

  // Free stuff
  rsstfreenewtorrentscontainer(newtorrents);
  free(searchValue);
}

void searchControl::tableDownload(int row, int )
{
  simpleCell *scell=NULL;
  int	id=0;

  // Get SwarmTv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Get the id
  scell = (simpleCell*) ui->searchTableWidget->cellWidget(row, 0);

  // Initiate download
  id=scell->getSimpleId();

  int ret = QMessageBox::warning(this, tr("SwarmTv"),
                                 tr("Okay to download?"),
                                 QMessageBox::Ok | QMessageBox::Cancel);
  if(ret == QMessageBox::Ok) {
    std::cout << "Id to download: " << id << std::endl;
    rsstdownloadbyid(swarm->getHandle(), id);
  }
}

void searchControl::initHeaders()
{
  // Make sure ui is set
  if(ui == NULL){
    std::cerr << "Set ui before calling initHeaders " << __FILE__ << ":" << __LINE__ << std::endl;
  }

  // Set title as filling
  ui->searchTableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);

  // Set Size static
  ui->searchTableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

  // Set source static
  ui->searchTableWidget->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);

  // Set metatype static
  ui->searchTableWidget->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);
}

int searchControl::rowToId(int row)
{
  simpleCell *scell=NULL;
  int	id=0;

  // Get the id
  scell = (simpleCell*) ui->searchTableWidget->cellWidget(row, 0);

  // Initiate download
  id=scell->getSimpleId();

  return id;
}

void searchControl::showContextMenu(const QPoint& pos) // this is a slot
{
  int row=0;
  newTorrentFullInfoDialog *ntfid = NULL;

  // for most widgets
  QPoint globalPos = ui->searchTableWidget->mapToGlobal(pos);
  // for QAbstractScrollArea and derived classes you would use:
  // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

  QMenu myMenu;
  myMenu.addAction(tr("Full Info"));
  myMenu.addAction(tr("Download"));
  // ...

  QAction* selectedItem = myMenu.exec(globalPos);
  if (selectedItem)
  {
    // Get the selected row from the table
    row = ui->searchTableWidget->selectedRanges().first().topRow();

    // Identify the different items clicked
    if(selectedItem->text().compare("Full Info") == 0){
      std::cout << "Full Info Clicked." << std::endl;
      ntfid = new newTorrentFullInfoDialog(this->rowToId(row));
      ntfid->show();
    }
    else if(selectedItem->text().compare("Download") == 0){
      std::cout << "Download clicked." << std::endl;
      this->tableDownload(row, 0);
    }

    // something was chosen, do stuff
  }
  else
  {
    // nothing was chosen
    }
}
