#include <QLabel>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

extern "C" {
#include <swarmtv.h>
}

#include <iostream>

#include "mainwindow.h"
#include "simpletablecontrol.hpp"
#include "simpleeditdialog.hpp"
#include "singleton.h"
#include "swarmtv.hpp"
#include "simplecell.hpp"

simpleTableControl::simpleTableControl(QWidget *parent) :
    QWidget(parent)
{
}

void simpleTableControl::setTable(QTableWidget *table)
{
    // Store the object we are going to control
    this->table = table;
}

int simpleTableControl::updateTable()
{
    int rc=0;
    lastdowned_container container;
    swarmTv *swarm = &Singleton<swarmTv>::Instance();

    // Set Header size policies
    simpleTableControl::initHeaders();

    // Get the last downloaded data
    rc = rsstgetlastdownloaded(swarm->getHandle() , &container);
    if(rc != 0) {
      fprintf(stderr, "Could not retrieve last downloaded data.\n");
      return -1;
    }

    // Fill the table
    fillTable(&container);

    // Clean up
    rsstfreelastdownedcontainer(&container);

    return 0;
}

void simpleTableControl::initHeaders()
{
  // Make sure ui is set
  if(this->table == NULL){
    std::cerr << "Set table before calling initHeaders " << __FILE__ << ":" << __LINE__ << std::endl;
  }

  // Set name
  table->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);

  // Set Filter Type
  table->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

  // Set Last downloaded Title
  table->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
}

void simpleTableControl::fillTable(lastdowned_container *container)
{
  int 		count=0;
  simpleCell 	*slabel=NULL;
  QLabel 		*label=NULL;

  /*
     * Call method to set
     */
  table->setRowCount(container->nr);

  /*
     * Loop through the content, and set the names + last download to the screen
     */
  for(count=0; count < container->nr; count++){
    /*
         * Set filtername in first table row
         */
    slabel = new simpleCell(container->lastdownloaded[count].filtername);
        slabel->setSimpleId(container->lastdownloaded[count].filterid);
        table->setCellWidget(count, 0, slabel);
        slabel = NULL;

        /*
         * Set filter type in second row.
         */
        label = new QLabel(container->lastdownloaded[count].filtertype);
        table->setCellWidget(count, 1, label);
        label=NULL;

        /*
         * Set lastdownloaded content in the third row
         */
        if(container->lastdownloaded[count].downloaded == NULL) {
            label = new QLabel("--");
        } else {
            label = new QLabel(container->lastdownloaded[count].downloaded->title);
        }
        table->setCellWidget(count, 2, label);
        label = NULL;
    }
}

void simpleTableControl::cellDoubleClicked(int row, int column)
{
    int 		id=0;
    simpleCell *scell=NULL;
    simpleEditDialog *dialog=NULL;

    // Get cell cell pointer
    scell = static_cast<simpleCell*>( table->cellWidget(row, 0) );

    // Exctract Id
    id = scell->getSimpleId();

    // Open dialog, and disable the name field.
    dialog = new simpleEditDialog(id, this);
    dialog->enableNameEnable(false);
    dialog->show();
}

void simpleTableControl::addSimpleButtonClicked()
{
    simpleEditDialog *dialog=NULL;

    // Open dialog, and disable the name field.
    dialog = new simpleEditDialog(this);
    dialog->show();
}

int simpleTableControl::getRowSelected()
{
  int row=0;

  // Get line that is currently selected
  QList<QTableWidgetSelectionRange> items = table->selectedRanges();
  if(items.count() == 0) {
    QMessageBox::warning(this, tr("SwarmTv"),
                         tr("Please select a source to delete."),
                         QMessageBox::Ok);
    return -1;
  }
  row = items.first().topRow();

  return row;
}

void simpleTableControl::editSimpleButtonClicked()
{
  int row=0;

  // Get selected row
  row = getRowSelected();
  if(row == -1) {
    return;
  }

  // Handle this through the double click handle
  cellDoubleClicked(row, 0);
}

void simpleTableControl::delSimpleButtonClicked()
{
  int row=0;
  int	id=0;
  int ret=0;
  simpleCell *scell=NULL;

  // Get SwarmTv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Get id from this row
  row = getRowSelected();
  if(row == -1) {
    return;
  }

  // Get cell cell pointer
  scell = static_cast<simpleCell*>( table->cellWidget(row, 0) );

  // Exctract Id
  id = scell->getSimpleId();

  // Ask for comformation
  ret = QMessageBox::warning(this, tr("SwarmTv"),
                                 tr("Are you sure you want to delete this filter?"),
                                 QMessageBox::Ok | QMessageBox::Cancel);

  if(ret == QMessageBox::Ok){
    // Delete the simple filter
    rsstdelsimpleid(swarm->getHandle(), id);

    // Update the view
    this->updateTable();
  }
}
