#ifndef SERIESWIDGET_HPP
#define SERIESWIDGET_HPP

#include <stdlib.h>

extern "C" {
#include <tvdb.h>
}

#include <QWidget>
#include <QLabel>
#include <QPixmap>

class seriesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit seriesWidget(QWidget *parent = 0);
    explicit seriesWidget(tvdb_series_t *series, tvdb_buffer_t *banner, QWidget *parent);

    QString *getTitle();

    ~seriesWidget();
signals:

public slots:

private:
    void createLayout();

    QLabel *bannerImage; // Image when available
    QLabel *title; // Title of the serie
    QLabel *overview; // Overview description
    QLabel *firstaired; // First aired
    tvdb_buffer_t *banner; // Buffer holding the banner image
};

#endif // SERIESWIDGET_HPP
