#ifndef SERIESWIDGET_HPP
#define SERIESWIDGET_HPP

#include <stdlib.h>

extern "C" {
#include <tvdb.h>
}

#include <QWidget>
#include <QLabel>
#include <QPixmap>
//#include "getepisodebanner.hpp"

class seriesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit seriesWidget(QWidget *parent = 0);
    explicit seriesWidget(tvdb_series_t *series, char *banner, QWidget *parent);

    QString *getTitle();

    ~seriesWidget();
signals:

public slots:
    void imageReady(tvdb_buffer_t *banner);
    void imageFailed(void);

private:
    void createLayout();

    QLabel *bannerImage; // Image when available
    QLabel *title; // Title of the serie
    QLabel *overview; // Overview description
    QLabel *firstaired; // First aired
    char *bannerName; // Buffer holding the banner image
    //getEpisodeBanner *getBan; // QThread object to get Image from Tvdb
    tvdb_buffer_t *imageBuffer; // Image buffer
};

#endif // SERIESWIDGET_HPP
