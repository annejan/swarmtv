#include "getseriestask.hpp"
#include <stdlib.h>

extern "C" {
  #include <tvdb.h>
}

#include "thetvdb.hpp"
#include "singleton.h"

getSeriesTask::getSeriesTask(char *query, QObject *parent) :
    QObject(parent)
{
  // xml buffer
  memset(&series_xml, 0, sizeof(tvdb_buffer_t));

  // store query
  this->query = strdup(query);
}

getSeriesTask::~getSeriesTask()
{
  // Cleanup
  tvdb_free_buffer(&series_xml);
  free(query);
}

void getSeriesTask::start()
{
  theTvdb *tvdb = &Singleton<theTvdb>::Instance();

  // Get XML results
  tvdb_series(tvdb->getTvdb(), query, "en", &series_xml);

  // emit signal holding results
  emit results(&series_xml);
}
