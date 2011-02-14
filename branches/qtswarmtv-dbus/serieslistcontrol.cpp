#include <iostream>
#include <stdlib.h>
#include <QApplication>
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

const QString max_string("700mb");
const QString min_string("300MB");

seriesListControl::seriesListControl(QWidget *parent) :
    QWidget(parent)
{
  tasks = new taskQueue();
}

seriesListControl::seriesListControl(QWidget *parent, QLineEdit *searchLine, QListWidget *list) :
    QWidget(parent)
{
  this->list = list;
  this->searchLine = searchLine;
  tasks = new taskQueue();
}

seriesListControl::~seriesListControl()
{
  delete(tasks);
}

void seriesListControl::setSeriesSearchLine(QLineEdit *searchLine)
{
  this->searchLine = searchLine;
}

void seriesListControl::setSeriesListWidget(QListWidget *list)
{
  this->list = list;

  QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
}

void seriesListControl::addWidget(tvdb_series_t *series)
{
  QString title(series->name);
  QString overview(series->overview);

  //MyItem is a custom widget that takes two strings and sets two labels to those string.
  seriesWidget *myItem = new seriesWidget(series, series->banner, tasks, list);
  QListWidgetItem *item = new QListWidgetItem();
  item->setSizeHint(QSize(0,180));
  this->list->addItem(item);
  this->list->setItemWidget(item, myItem);
  myItem->show();
}

void seriesListControl::handleSeries(tvdb_list_front_t *series)
{
  tvdb_list_node_t *n=NULL;
  tvdb_series_t *m=NULL;

  tvdb_list_reset(series);

  n=tvdb_list_next(series);
  while(n != NULL) {
    m = (tvdb_series_t *)n->data;
    //this->list->addItem(m->overview);
    addWidget(m);
    n=tvdb_list_next(series);
  }
}

void seriesListControl::findSeries()
{
  getSeriesTask *st=NULL;

  // Disable line edit
  searchLine->setEnabled(false);

  // Wait for running task to finish then empty queue
  tasks->clearTasks();
  list->clear();

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

void seriesListControl::itemDoubleClicked(QListWidgetItem *item)
{
  seriesWidget *series=NULL;
  // Get widget pointer
  series = dynamic_cast<seriesWidget*>(list->itemWidget(item));

  // Print name for now
  std::cout << "Name double clicked: " << series->getTitle()->toUtf8().begin() << std::endl;

  // Create new simple filter window
  openSimpleEditDialog(series);
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
  dialog = new simpleEditDialog(list);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setTitle(&title);
  dialog->setName(&name);
  dialog->setMaxSize((QString*)&max_string);
  dialog->setMinSize((QString*)&min_string);
  dialog->enableNameEnable(true);
  dialog->show();
}
