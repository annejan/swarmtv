#include "seasonepisodewidget.hpp"
#include "ui_seasonepisodewidget.h"
extern "C" {
  #include "tvdb.h"
}
#include "QSettings"
#include "QDebug"
#include "QListWidgetItem"

static const QString TVDB_API_CONFIG("config/tvdbapiconfig");

seasonEpisodeWidget::seasonEpisodeWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::seasonEpisodeWidget)
{
    ui->setupUi(this);
}

seasonEpisodeWidget::~seasonEpisodeWidget()
{
    delete ui;
}

void seasonEpisodeWidget::setSeriesTitle(QString &name)
{
  seriesName=name;

  // Set series name in window
  ui->nameLabel->setText(seriesName);
}

void seasonEpisodeWidget::setSeriesId(int id)
{
  seriesId=id;
}

void seasonEpisodeWidget::retrieveEpisodeData()
{
  QSettings settings;
  QString api_key;
  htvdb_t tvdb=0;
  int rc=0;
  tvdb_buffer_t buf;
  tvdb_list_front_t seriesInfo;

  // Initialize structs
  memset(&buf, 0, sizeof(tvdb_buffer_t));
  memset(&seriesInfo, 0, sizeof(tvdb_list_front_t));

  // Get the API-key
  api_key = settings.value(TVDB_API_CONFIG).toString();

  // create a new tvdb handle (not to get problems with threads)
  tvdb = tvdb_init((char *) api_key.toUtf8().data());

  // Execute query
  rc = tvdb_series_info(tvdb, seriesId, "en", &buf);
  if(rc != TVDB_OK){
    qDebug() << "tvdb_series_info returned not okay.";
    return;
  }

  // Parse the XML data
  rc = tvdb_parse_series_info(&buf, "", &seriesInfo);
  if(rc != TVDB_OK){
    qDebug() << "tvdb_parse_series_info returned not okay.";
  }

  // Add the data to the ListView
  fillListView(&seriesInfo);

  // Detroy the tvdb instance
  tvdb_list_remove(&seriesInfo);
  tvdb_uninit(tvdb);
}

void seasonEpisodeWidget::addSeason()
{

}

void seasonEpisodeWidget::addEpisode()
{

}

void seasonEpisodeWidget::fillListView(tvdb_list_front_t *seriesInfo)
{
  tvdb_list_reset(seriesInfo);
  const tvdb_list_node_t *n=NULL;
  tvdb_series_info_t *s=NULL;
  int widgetSeason=0;
  int curSeason=0;
  int curEpisode=0;
  QString episodeName;
  QString seasonName;
  QListWidgetItem *seasonItem = NULL;
  QListWidgetItem *episodeItem = NULL;

  n = tvdb_list_next(seriesInfo);
  while(n != NULL) {
    s = (tvdb_series_info_t *)n->data;
    curSeason=s->season_number;
    curEpisode=s->episode_number;

    if(curSeason != 0 && curEpisode != 0){
      // Add season entry
      if(curSeason > widgetSeason){
        qDebug() << "Add season: " << curSeason;

        // Create new season entry
        seasonItem = new QListWidgetItem();
        seasonName.sprintf("Season %d", curSeason);
        seasonItem->setText(seasonName);
        ui->listWidget->addItem(seasonItem);

        // Set new season
        widgetSeason=curSeason;
      }
      if(seasonItem != NULL) {
        // Add episode entry
        episodeItem = new QListWidgetItem(*seasonItem);
        episodeName.sprintf("%d - %s", s->episode_number, s->episode_name);
        episodeItem->setText(episodeName);
        ui->listWidget->addItem(episodeItem);

        qDebug() << "	id [" << s->id << "], seriesid [" << s->series_id << "], name [" << s->episode_name <<
            "] episode [" << s->episode_number << "]";
      }

    }

    n = tvdb_list_next(seriesInfo);
  }
}
