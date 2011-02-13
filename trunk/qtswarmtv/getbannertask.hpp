#ifndef GETBANNERTASK_HPP
#define GETBANNERTASK_HPP

#include <QObject>
#include "taskinterface.hpp"

extern "C" {
#include "tvdb.h"
}

class getBannerTask : public QObject, public taskInterface
{
  Q_OBJECT
public:
  explicit getBannerTask(QObject *parent = 0);
  explicit getBannerTask(char* filename, QObject *parent = 0);
  ~getBannerTask();

  void start(void);

  void setFilename(char *filename);
signals:
  void bannerReady(tvdb_buffer_t *tvdb);
  void bannerFailed();

public slots:

private:
  char *filename;
  tvdb_buffer_t bannerData;
};

#endif // GETBANNERTASK_HPP
