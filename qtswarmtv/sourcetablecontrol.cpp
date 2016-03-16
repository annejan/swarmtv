#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <iostream>
#include <swarmtv.h>

#include "sourcetablecontrol.hpp"
#include "sourceeditdialog.hpp"
#include "singleton.h"
#include "swarmtv.hpp"
#include "simplecell.hpp"

sourceTableControl::sourceTableControl(QWidget *parent) :
    QWidget(parent)
{
}


void sourceTableControl::setTable(QTableWidget *table)
{
    // Store the object we are going to control
    this->table = table;
}


int sourceTableControl::updateTable()
{
    int rc=0;
    source_container *container;
    swarmTv *swarm = &Singleton<swarmTv>::Instance();

    /*
     * Get the last downloaded data
     */
    rc = rsstgetallsources(swarm->getHandle(), &container);
    if(rc != 0) {
      fprintf(stderr, "Could not retrieve last downloaded data.\n");
      return -1;
    }

    /*
     * Fill the table
     */
    fillTable(container);

    /*
     * Clean up
     */
    rsstfreesourcecontainer(container);

    return 0;
}

void sourceTableControl::initHeaders()
{
  // Make sure ui is set
  if(this->table == NULL){
    std::cerr << "Set table before calling initHeaders " << __FILE__ << ":" << __LINE__ << std::endl;
  }

  // Name
#if QT_VERSION >= 0x050000
  table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
#else
  table->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
#endif
  // Parser
#if QT_VERSION >= 0x050000
  table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
  table->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
#endif

  // Meta type
#if QT_VERSION >= 0x050000
  table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
#else
  table->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
#endif

  // URL
#if QT_VERSION >= 0x050000
  table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
#else
  table->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);
#endif
}

void sourceTableControl::fillTable(source_container *container)
{
    int 		count=0;
    simpleCell 	*slabel=NULL;
    QLabel 		*label=NULL;

    // Set Header size hints
    initHeaders();

    // Call method to set
    table->setRowCount(container->nr);

    // Loop through the content, and set the names + last download to the screen
    for(count=0; count < container->nr; count++){
        // Set filtername in first table row
        slabel = new simpleCell(container->source[count].name);
        slabel->setSimpleId(container->source[count].id);
        table->setCellWidget(count, 0, slabel);
        slabel = NULL;

        // Set filter type in second row.
        label = new QLabel(container->source[count].parser);
        table->setCellWidget(count, 1, label);
        label=NULL;

        // Set filter type in second row.
        label = new QLabel(container->source[count].metatype);
        table->setCellWidget(count, 2, label);
        label=NULL;

        // Set filter type in second row.
        label = new QLabel(container->source[count].url);
        table->setCellWidget(count, 3, label);
        label=NULL;
    }

}

void sourceTableControl::cellDoubleClicked(int row, int )
{
    int id=0;
    simpleCell 	*scell=NULL;
    sourceEditDialog *dialog=NULL;

    // Get cell cell pointer
    scell = static_cast<simpleCell*>( table->cellWidget(row, 0) );
    id = scell->getSimpleId();

    // Create object of source containing source
    dialog = new sourceEditDialog(id, this);
    dialog->setNameEnable(false);
    dialog->show();

    // Print something.
    std::cout << "Row clicked : " << row << " ID : " << id << std::endl;
}

void sourceTableControl::addButtonClicked(void)
{
    sourceEditDialog *dialog=NULL;

    // Create object of source containing source
    dialog = new sourceEditDialog(this);
    dialog->show();
}


void sourceTableControl::delButtonClicked(void)
{
  int row=0;
  int id=0;
  int rc=0;
  QString name;
  simpleCell 	*scell=NULL;
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Get line that is currently selected
  QList<QTableWidgetSelectionRange> items = table->selectedRanges();
  if(items.count() == 0) {
    QMessageBox::warning(this, tr("SwarmTv"),
                         tr("Please select a source to delete."),
                         QMessageBox::Ok);

    return;
  }
  row = items.first().topRow();

  // Get cell cell pointer
  scell = static_cast<simpleCell*>( table->cellWidget(row, 0) );
  id = scell->getSimpleId();
  name = scell->text();

  // Show dialog, asking for permission to delete
  int ret = QMessageBox::warning(this, tr("SwarmTv"),
                                 tr("Are you sure you want to delete this source?"),
                                 QMessageBox::Ok | QMessageBox::Cancel);
  switch(ret) {
        case QMessageBox::Ok:
    //@DEBUG
#if QT_VERSION >= 0x050000
    std::cout << "deleting source '" << name.toLatin1().data() << "." << std::endl;
#else
    std::cout << "deleting source '" << name.toAscii().data() << "." << std::endl;
#endif
    rc = rsstdelsourceid(swarm->getHandle(), id);
    if(rc != 0) {
      QMessageBox::warning(this, tr("SwarmTv"),
                           tr("Deleting of source failed!"),
                           QMessageBox::Ok);
    }
    break;
        case QMessageBox::Cancel:

    break;
        default:
    // When this happens, weird things are going on.
    exit(1);
  }

  // Delete the Filter when accepted

  // Update display
  this->updateTable();
}
