#ifndef SERIESLISTCONTROL_HPP
#define SERIESLISTCONTROL_HPP

#include <stdlib.h>

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
extern "C" {
#include <tvdb.h>
}

class seriesListControl : public QWidget
{
    Q_OBJECT
public:
    explicit seriesListControl(QWidget *parent=0);
    explicit seriesListControl(QWidget *parent, QLineEdit *searchLine, QListWidget *list);
    void setSeriesSearchLine(QLineEdit *searchLine);
    void setSeriesListWidget(QListWidget *list);
signals:

public slots:
    void findSeries();

private:
    void handleSeries(tvdb_list_front_t *series);
    void addWidget(tvdb_series_t *series);

    QListWidget *list;
    QLineEdit *searchLine;
};

#endif // SERIESLISTCONTROL_HPP
