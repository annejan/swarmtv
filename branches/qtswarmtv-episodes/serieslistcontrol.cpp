#include <iostream>
#include <stdlib.h>
#include <QApplication>
#include <QMenu>
#include <QDebug>
extern "C" {
#include <tvdb.h>
}
#include "singleton.h"
#include "serieslistcontrol.hpp"
#include "serieswidget.hpp"
#include "singleton.h"
#include "thetvdb.hpp"
#include "simpleeditdialog.hpp"
#include "getseriestask.hpp"
#include "seasonepisodewidget.hpp"

const QString max_string("700mb");
const QString min_string("300MB");

seriesListControl::seriesListControl(QWidget *parent) :
    QWidget(parent)
{
  tasks = new taskQueue();
}

seriesListControl::seriesListControl(QWidget *parent, QLineEdit *searchLine, QTableWidget *table) :
    QWidget(parent)
{
  this->table = table;
  this->searchLine = searchLine;
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  tasks = new taskQueue();

  qDebug() << "Connected through constructor.";
  QObject::connect(table, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                   this, SLOT(itemDoubleClicked(QListWidgetItem*)));
  QObject::connect(table, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(showContextMenu(QPoint)));
}

seriesListControl::~seriesListControl()
{
  delete(tasks);
}

void seriesListControl::setSeriesSearchLine(QLineEdit *searchLine)
{
  this->searchLine = searchLine;
}

void seriesListControl::setSeriesTableWidget(QTableWidget *table)
{
  this->table = table;

  qDebug() << "Connected through setList.";
  QObject::connect(table, SIGNAL(doubleClicked(QModelIndex)),
                   this, SLOT(itemDoubleClicked()));
  QObject::connect(table, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(showContextMenu(QPoint)));
}

void seriesListControl::addWidget(int count, tvdb_series_t *series)
{
  QString title(series->name);
  QString overview(series->overview);

  seriesWidget *myItem = new seriesWidget(series, series->banner, tasks, series->series_id, table);
  myItem->setMinimumHeight(180);
  table->setCellWidget(count, 0, myItem);
  table->setRowHeight(count, myItem->height()+25);
  myItem->show();
}

void seriesListControl::handleSeries(tvdb_list_front_t *series)
{
  int count=0;
  tvdb_list_node_t *n=NULL;
  tvdb_series_t *m=NULL;

  table->setRowCount(series->count);

  tvdb_list_reset(series);

  n=tvdb_list_next(series);
  while(n != NULL) {
    m = (tvdb_series_t *)n->data;
    //this->list->addItem(m->overview);
    addWidget(count, m);
    n=tvdb_list_next(series);
    count++;
  }
}

void seriesListControl::findSeries()
{
  getSeriesTask *st=NULL;

  // Disable line edit
  searchLine->setEnabled(false);

  // Wait for running task to finish then empty queue
  tasks->clearTasks();
  //list->clear();
  table->clear();

  // Create and initialize query task
  st = new getSeriesTask(this->searchLine->text().toUtf8().begin(), this);
  QObject::connect(st, SIGNAL(results(tvdb_buffer_t*)), this, SLOT(searchResults(tvdb_buffer_t*)));

  // Start task
  tasks->addTask(st);
}

void seriesListControl::searchResults(tvdb_buffer_t *series_xml)
{
  int rc=0;
  tvdb_list_front_t series;

  // Parse results
  tvdb_list_init(&series);
  rc = tvdb_parse_series(series_xml, 0, &series);
  if(rc == TVDB_OK) {
    // Insert text for now
    handleSeries(&series);
  }

  // Set searchbar to editable again
  searchLine->setEnabled(true);
}

void seriesListControl::itemDoubleClicked()
{
  createFilter();
}

void seriesListControl::openSimpleEditDialog(seriesWidget *series)
{
  simpleEditDialog *dialog=NULL;
  QString title;
  QString name;

  // Get strings
  title = series->getTitle()->toUtf8();
  name = series->getTitle()->toUtf8();

  // Open dialog, and disable the name field.
  dialog = new simpleEditDialog(table);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setTitle(&title);
  dialog->setName(&name);
  dialog->setMaxSize((QString*)&max_string);
  dialog->setMinSize((QString*)&min_string);
  dialog->enableNameEnable(true);
  dialog->show();
}

void seriesListControl::showContextMenu(const QPoint& pos) // this is a slot
{
  int row=0;

  //newTorrentFullInfoDialog *ntfid = NULL;
  qDebug() << "Context menu called.";

  // for most widgets
  QPoint globalPos = table->mapToGlobal(pos);
  // for QAbstractScrollArea and derived classes you would use:
  // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
  if(table->selectedRanges().isEmpty() == true) {
    return;
  }

  QMenu myMenu;
  myMenu.addAction(tr("Create filter"));
  myMenu.addAction(tr("Get Episode info"));
  // ...

  QAction* selectedItem = myMenu.exec(globalPos);
  if (selectedItem)
  {
    // Get the selected row from the table
    row = table->selectedRanges().first().topRow();

    // Identify the different items clicked
    if(selectedItem->text().compare("Create filter") == 0){
      qDebug() << "Create Filter Selected";
      createFilter();
      //ntfid = new newTorrentFullInfoDialog(this->rowToId(row));
      //ntfid->show();
    }
    else if(selectedItem->text().compare("Get Episode info") == 0){
      qDebug() << "Get episode info clicked.";
      showEpisodes();
    }

    // something was chosen, do stuff
  }
  else
  {
    // nothing was chosen
  }
}

void seriesListControl::createFilter()
{
  seriesWidget *series=NULL;
  int selectedRow=0;

  selectedRow = table->selectedRanges().first().topRow();
  series = dynamic_cast<seriesWidget*>(table->cellWidget(selectedRow, 0));

  // Create new simple filter window
  openSimpleEditDialog(series);
}

void seriesListControl::showEpisodes()
{
  seriesWidget *series=NULL;
  int selectedRow=0;

  selectedRow = table->selectedRanges().first().topRow();
  series = dynamic_cast<seriesWidget*>(table->cellWidget(selectedRow, 0));

  seasonEpisodeWidget *seasonEpisode = new seasonEpisodeWidget(table);
  seasonEpisode->setSeriesTitle(*(series->getTitle()));
  seasonEpisode->setSeriesId(series->getSeriesId());
  seasonEpisode->retrieveEpisodeData();
  seasonEpisode->show();
}
