#include <QTableWidget>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

#include <sqlite3.h>
#include <stdlib.h>
#include <iostream>
#include <swarmtv.h>

#include "swarmtv.hpp"
#include "singleton.h"
#include "downloadedtablecontrol.hpp"
#include "simplecell.hpp"


#define DOWNLOADED_LIMIT 150

downloadedTableControl::downloadedTableControl(QWidget *parent) :
    QWidget(parent)
{
}

void downloadedTableControl::setTable(QTableWidget *downtable)
{
  this->downtable = downtable;

  initHeaders();
}


int downloadedTableControl::getRowSelected()
{
  int row=0;

  // Get line that is currently selected
  QList<QTableWidgetSelectionRange> items = downtable->selectedRanges();
  if(items.count() == 0) {
    QMessageBox::warning(this, tr("SwarmTv"),
                         tr("Please select a downloaded item to delete."),
                         QMessageBox::Ok);
    return -1;
  }
  row = items.first().topRow();

  return row;
}


void downloadedTableControl::fillTable()
{
  int rc=0;
  int count=0;
  downloaded_container *downed=NULL;
  QLabel *cell=NULL;
  simpleCell *scell=NULL;
  char *text=NULL;
  int id=0;

  // Get Swarmtv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Fill cells
  rc = rsstgetdownloaded(swarm->getHandle(), &downed, DOWNLOADED_LIMIT, 0);
  if(rc == 0) {

    // Set rows in table
    downtable->setRowCount(downed->nr);

    while(count < downed->nr){
      id=downed->downloaded[count].id;
      text=downed->downloaded[count].title;
      scell = new simpleCell(text);
      scell->setSimpleId(id);
      downtable->setCellWidget(count, 0, scell);

      text = downed->downloaded[count].downdate;
      cell = new QLabel(text);
      downtable->setCellWidget(count, 1, cell);

      text = downed->downloaded[count].metatype;
      cell = new QLabel(text);
      downtable->setCellWidget(count, 2, cell);

      count++;
    }

    // clean up
    rsstfreedownloadedcontainer(downed);
  }
}

void downloadedTableControl::initHeaders()
{
  // Set title as filling
  downtable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);

  // Set Size static
  downtable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

  // Set source static
  downtable->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
}

void downloadedTableControl::delClicked()
{
  simpleCell *scell=NULL;
  int id=0;
  int row=0;
  int rc=0;

  // Get Swarmtv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  /*
   * Get the row selected row.
   * When nothing selected do nothing
   */
  row = getRowSelected();
  if(row == -1){
    return;
  }
  scell = static_cast<simpleCell*>(downtable->cellWidget(row, 0));
  id = scell->getSimpleId();

  /*
   * Delete the entry from the downloaded table.
   */
  rc = rsstdeldownloaded(swarm->getHandle(), id);

  /*
   * Update table contents
   */
  this->fillTable();
}

void downloadedTableControl::cellDoubleClicked(int,int)
{
  int ret=0;

  // Ask for comformation
  ret = QMessageBox::warning(this, tr("SwarmTv"),
                                 tr("Delete this entry from the downloaded record?"),
                                 QMessageBox::Ok | QMessageBox::Cancel);

  if(ret == QMessageBox::Ok){
    // Update the view
    this->delClicked();
  }


}
