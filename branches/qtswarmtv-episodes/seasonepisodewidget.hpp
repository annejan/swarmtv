#ifndef SEASONEPISODEWIDGET_HPP
#define SEASONEPISODEWIDGET_HPP

#include <QDialog>
extern "C" {
#include <tvdb.h>
}

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
    void addSeason();
    void addEpisode();

private:
    Ui::seasonEpisodeWidget *ui;
    QString seriesName;
    int seriesId;
};

#endif // SEASONEPISODEWIDGET_HPP
