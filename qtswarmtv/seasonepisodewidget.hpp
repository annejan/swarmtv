#ifndef SEASONEPISODEWIDGET_HPP
#define SEASONEPISODEWIDGET_HPP

#include <QDialog>
extern "C" {
#include <tvdb.h>
}
#include <QTreeWidget>
#include <taskqueue.hpp>

class episodeInfoWidget;

namespace Ui {
    class seasonEpisodeWidget;
}

class seasonEpisodeWidget : public QDialog
{
    Q_OBJECT

public:
    explicit seasonEpisodeWidget(QWidget *parent = 0);
    ~seasonEpisodeWidget();
    void setSeriesTitle(QString &name);
    void setSeriesId(int id);
    void setrieveEpisodeData();
    void fillListView(tvdb_list_front_t *);
    void retrieveEpisodeData();

public slots:
    // GUI signals
    void itemExpanded(QTreeWidgetItem *item);
    void itemDoubleClicked(QTreeWidgetItem *item, int column);

    // Task signals
    void seriesResults(tvdb_buffer_t *series_xml);
    void seriesFailed();

private:
    void addTask(episodeInfoWidget *widget);
    QTreeWidgetItem *addSeasonEntry(int seasonNum);
    void addEpisodeEntry(QTreeWidgetItem *season, tvdb_series_info_t *s);

    Ui::seasonEpisodeWidget *ui;
    QString seriesName;
    int seriesId;
    taskQueue tc;
    htvdb_t tvdb;
};

#endif // SEASONEPISODEWIDGET_HPP
