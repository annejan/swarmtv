#ifndef SERIESLISTCONTROL_HPP
#define SERIESLISTCONTROL_HPP

#include <stdlib.h>

#include <QWidget>
#include <QTableWidget>
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
    explicit seriesListControl(QWidget *parent, QLineEdit *searchLine, QTableWidget *table);
    ~seriesListControl();
    void setSeriesSearchLine(QLineEdit *searchLine);
    void setSeriesTableWidget(QTableWidget *table);
signals:

public slots:
    void findSeries();
    void searchResults(tvdb_buffer_t *series_xml);
    void itemDoubleClicked();
    void showContextMenu(const QPoint&);

private:
    void handleSeries(tvdb_list_node_t *series);
    void addWidget(int count, tvdb_series_t *series);
    void openSimpleEditDialog(seriesWidget *series);
    void createFilter();
    void showEpisodes();
    void nameToRegexp(const QString &name, QString &regexp);

    QTableWidget *table;
    QLineEdit *searchLine;
    taskQueue *tasks;
};

#endif // SERIESLISTCONTROL_HPP
