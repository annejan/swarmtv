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
  this->filename=NULL;
  memset(&bannerData, 0, sizeof(tvdb_buffer_t));
}

getBannerTask::getBannerTask(char *filename, QObject *parent) :
    QObject(parent)
{
  std::cout << "Filename set: " << filename << std::endl;
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

  std::cout << "Filename set: " << filename << std::endl;
  this->filename = strdup(filename);
}

void getBannerTask::start(void)
{
  int rc=NULL;
  theTvdb *htvdb = &Singleton<theTvdb>::Instance();

  std::cout << "Task started, getting: " << filename << std::endl;
  rc = tvdb_banners(htvdb->getTvdb(), filename, &bannerData);
  if(rc == TVDB_OK) {
    emit bannerReady(&bannerData);
  } else {
    emit bannerFailed();
  }
  std::cout << "Task done." << std::endl;
}

