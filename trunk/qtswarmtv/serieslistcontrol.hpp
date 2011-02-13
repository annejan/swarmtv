#ifndef SERIESLISTCONTROL_HPP
#define SERIESLISTCONTROL_HPP

#include <stdlib.h>

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
extern "C" {
#include <tvdb.h>
}

#include "taskqueue.hpp"

class seriesWidget;

class seriesListControl : public QWidget
{
    Q_OBJECT
public:
    explicit seriesListControl(QWidget *parent=0);
    explicit seriesListControl(QWidget *parent, QLineEdit *searchLine, QListWidget *list);
    ~seriesListControl();
    void setSeriesSearchLine(QLineEdit *searchLine);
    void setSeriesListWidget(QListWidget *list);
signals:

public slots:
    void findSeries();
    void searchResults(tvdb_buffer_t *series_xml);
    void itemDoubleClicked(QListWidgetItem *item);

private:
    void handleSeries(tvdb_list_front_t *series);
    void addWidget(tvdb_series_t *series);
    void openSimpleEditDialog(seriesWidget *series);

    QListWidget *list;
    QLineEdit *searchLine;
    taskQueue *tasks;
};

#endif // SERIESLISTCONTROL_HPP
