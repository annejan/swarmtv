#include "getseriesinfotask.hpp"
#include <stdlib.h>
extern "C" {
  #include <tvdb.h>
}

getSeriesInfoTask::getSeriesInfoTask(htvdb_t handle, int seriesId, QObject *parent) :
    QObject(parent)
{
  // Set the tvdb handle
  this->handle = handle;

  // Set the url that is to be retrieved
  this->seriesId = seriesId;

  // Initialize structs
  memset(&series_xml, 0, sizeof(series_xml));
}

getSeriesInfoTask::~getSeriesInfoTask()
{
  tvdb_free_buffer(&series_xml);
}


void getSeriesInfoTask::start()
{
  int rc=0;

  // retrieve data from libtvdb
  rc = tvdb_series_info(handle, seriesId, "en", &series_xml);
  if(rc != TVDB_OK){
    emit  failed();
  } else {
    // emit signal holding results
    emit results(&series_xml);
  }
}
