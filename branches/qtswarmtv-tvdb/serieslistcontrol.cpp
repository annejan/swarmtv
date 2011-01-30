#include <iostream>
#include <stdlib.h>
extern "C" {
#include <tvdb.h>
}
#include "singleton.h"
#include "serieslistcontrol.hpp"
#include "serieswidget.hpp"
#include "singleton.h"
#include "thetvdb.hpp"

seriesListControl::seriesListControl(QWidget *parent) :
    QWidget(parent)
{
}

seriesListControl::seriesListControl(QWidget *parent, QLineEdit *searchLine, QListWidget *list) :
    QWidget(parent)
{
  this->list = list;
  this->searchLine = searchLine;
}

void seriesListControl::setSeriesSearchLine(QLineEdit *searchLine)
{
  this->searchLine = searchLine;
}

void seriesListControl::setSeriesListWidget(QListWidget *list)
{
  this->list = list;
}

void seriesListControl::addWidget(tvdb_series_t *series)
{
  int rc=0;
  QString title(series->name);
  QString overview(series->overview);
  tvdb_buffer_t bannerBuffer;
  theTvdb *tvdb = &Singleton<theTvdb>::Instance();

  memset(&bannerBuffer, 0, sizeof(tvdb_buffer_t));

  // Get image from tvdb server
  rc = tvdb_banners(tvdb->getTvdb(), series->banner, &bannerBuffer);
  if(rc != TVDB_OK) {
    memset(&bannerBuffer, 0, sizeof(tvdb_buffer_t));
  }

  //MyItem is a custom widget that takes two strings and sets two labels to those string.
  seriesWidget *myItem = new seriesWidget(title, overview, &bannerBuffer, list);
  QListWidgetItem *item = new QListWidgetItem();
  item->setSizeHint(QSize(0,180));
  this->list->addItem(item);
  this->list->setItemWidget(item, myItem);
  myItem->show();

  tvdb_free_buffer(&bannerBuffer);
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
  int rc=0;
  tvdb_list_front_t series;
  tvdb_buffer_t series_xml;
  theTvdb *tvdb = &Singleton<theTvdb>::Instance();

  // Init tvdb
  memset(&series_xml, 0, sizeof(tvdb_buffer_t));

  // Do search
  tvdb_series(tvdb->getTvdb(), this->searchLine->text().toUtf8(), "en", &series_xml);

  // Clean ListWidget
  list->clear();

  // Parse results
  tvdb_list_init(&series);
  rc = tvdb_parse_series(&series_xml, 0, &series);
  if(rc == TVDB_OK) {
    // Insert text for now
    handleSeries(&series);
  }

  // Put resulting objects in QListView
  tvdb_free_buffer(&series_xml);
  tvdb_list_remove(&series);
}

