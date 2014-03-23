#ifndef SERIESWIDGET_HPP
#define SERIESWIDGET_HPP

#include <stdlib.h>

extern "C" {
#include <tvdb.h>
}

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include "taskqueue.hpp"

class seriesWidget : public QWidget
{
    Q_OBJECT
public:
    //explicit seriesWidget(QWidget *parent = 0);
    explicit seriesWidget(tvdb_series_t *series, char *banner, taskQueue *tasks, int seriesId, QWidget *parent);

    QString *getTitle();
    int getSeriesId();

    ~seriesWidget();
signals:

public slots:
    void imageReady(tvdb_buffer_t *banner);
    void imageFailed(void);

private:
    void createLayout();
    void addBannerTask(const QString &bannerString);

    QLabel *bannerImage; // Image when available
    QLabel *title; // Title of the serie
    QString *titleString; // Title text
    QLabel *overview; // Overview description
    QLabel *firstaired; // First aired
    int seriesId;			// ID of the series displayed
    char *bannerName; // Buffer holding the banner image
    tvdb_buffer_t *imageBuffer; // Image buffer
    taskQueue *tasks; // Task queue object
};

#endif // SERIESWIDGET_HPP
