#ifndef GETBANNERTASK_HPP
#define GETBANNERTASK_HPP

#include <QObject>
#include "taskinterface.hpp"

extern "C" {
#include "tvdb.h"
}

class theTvdb;

class getBannerTask : public QObject, public taskInterface
{
  Q_OBJECT
public:
  explicit getBannerTask(QObject *parent = 0);
  explicit getBannerTask(char* filename, QObject *parent = 0);
  explicit getBannerTask(char* filename, theTvdb *htvdb, QObject *parent = 0);
  explicit getBannerTask(char* filename, htvdb_t tvdb, QObject *parent = 0);
  ~getBannerTask();

  void start(void);
  void setFilename(char *filename);
  void setTvdbHandle(theTvdb *htvdb);
signals:
  void bannerReady(tvdb_buffer_t *tvdb);
  void bannerFailed();

public slots:

private:
  char *filename;
  tvdb_buffer_t bannerData;
  htvdb_t tvdb;
};

#endif // GETBANNERTASK_HPP
