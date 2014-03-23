#ifndef GETSERIESTASK_HPP
#define GETSERIESTASK_HPP

#include <QObject>
#include "taskinterface.hpp"
extern "C" {
  #include <tvdb.h>
}

class getSeriesTask : public QObject, public taskInterface
{
    Q_OBJECT
public:
    explicit getSeriesTask(char *query, QObject *parent = 0);
    ~getSeriesTask();
    void start();

signals:
    void results(tvdb_buffer_t *series_xml);

public slots:

private:
  tvdb_buffer_t series_xml;
  char *query;
};

#endif // GETSERIESTASK_HPP
