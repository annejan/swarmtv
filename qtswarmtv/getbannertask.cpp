#include <iostream>
#include <stdlib.h>

extern "C" {
#include <tvdb.h>
}

#include "singleton.h"
#include "getbannertask.hpp"
#include "thetvdb.hpp"

getBannerTask::getBannerTask(QObject *parent) :
    QObject(parent)
{
  theTvdb *htvdb=NULL;
  htvdb = &Singleton<theTvdb>::Instance();
  this->tvdb = htvdb->getTvdb();

  this->filename=NULL;
  memset(&bannerData, 0, sizeof(tvdb_buffer_t));
}

getBannerTask::getBannerTask(char *filename, QObject *parent) :
    QObject(parent)
{
  theTvdb *htvdb=NULL;
  this->tvdb = htvdb->getTvdb();

  this->filename=strdup(filename);
  memset(&bannerData, 0, sizeof(tvdb_buffer_t));
}

getBannerTask::getBannerTask(char *filename, theTvdb *htvdb, QObject *parent) :
    QObject(parent)
{
  this->tvdb=htvdb->getTvdb();
  this->filename=strdup(filename);
  memset(&bannerData, 0, sizeof(tvdb_buffer_t));
}

getBannerTask::getBannerTask(char* filename, htvdb_t tvdb, QObject *parent) :
    QObject(parent)
{
  this->tvdb=tvdb;
  this->filename=strdup(filename);
  memset(&bannerData, 0, sizeof(tvdb_buffer_t));
}

getBannerTask::~getBannerTask()
{
  free(filename);
  tvdb_free_buffer(&bannerData);
}

void getBannerTask::setFilename(char *filename)
{
  if(this->filename != NULL) {
    free(this->filename);
  }

  this->filename = strdup(filename);
}

void getBannerTask::setTvdbHandle(theTvdb *htvdb)
{
  this->tvdb = htvdb->getTvdb();
}

void getBannerTask::start(void)
{
  int rc=NULL;

  rc = tvdb_banners(tvdb, filename, &bannerData);
  if(rc == TVDB_OK) {
    emit bannerReady(&bannerData);
  } else {
    emit bannerFailed();
  }
}

