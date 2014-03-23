#ifndef GETSERIESINFOTASK_H
#define GETSERIESINFOTASK_H

#include <QObject>
#include "taskinterface.hpp"
extern "C" {
  #include <tvdb.h>
}

class getSeriesInfoTask : public QObject, public taskInterface
{
    Q_OBJECT
public:
  explicit getSeriesInfoTask(htvdb_t handle, int seriesId, QObject *parent = 0);
  ~getSeriesInfoTask();
  void start();

signals:
  void results(tvdb_buffer_t *series_xml);
  void failed();

public slots:

private:
  htvdb_t handle;
  tvdb_buffer_t series_xml;
  int seriesId;
};

#endif // GETSERIESINFOTASK_H
