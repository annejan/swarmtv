#include "seasonepisodewidget.hpp"
#include "ui_seasonepisodewidget.h"

extern "C" {
  #include "tvdb.h"
}

#include "QSettings"
#include "QDebug"
#include "QTreeWidget"

#include "episodeinfowidget.hpp"
#include "getbannertask.hpp"

static const QString TVDB_API_CONFIG("config/tvdbapiconfig");

seasonEpisodeWidget::seasonEpisodeWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::seasonEpisodeWidget)
{
  QString api_key;
  QSettings settings;

  ui->setupUi(this);

  // Create the tvdb instance for this window
  api_key = settings.value(TVDB_API_CONFIG).toString();
  tvdb = tvdb_init((char *) api_key.toUtf8().data());
}

seasonEpisodeWidget::~seasonEpisodeWidget()
{
  tc.stopTasks();

  tvdb_uninit(tvdb);
  delete ui;
}

void seasonEpisodeWidget::setSeriesTitle(QString &name)
{
  seriesName=name;

  // Set series name in window
  ui->nameLabel->setText(seriesName);

  // Connect the expand signal
  QObject::connect(ui->treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
}

void seasonEpisodeWidget::setSeriesId(int id)
{
  seriesId=id;
}

void seasonEpisodeWidget::retrieveEpisodeData()
{
  int rc=0;
  tvdb_buffer_t buf;
  tvdb_list_front_t seriesInfo;

  // Initialize structs
  memset(&buf, 0, sizeof(tvdb_buffer_t));
  memset(&seriesInfo, 0, sizeof(tvdb_list_front_t));


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
}

void seasonEpisodeWidget::addSeason()
{

}

void seasonEpisodeWidget::addEpisode()
{

}

void seasonEpisodeWidget::addTask(episodeInfoWidget *widget)
{
  getBannerTask *task = NULL;

  // Get the filename
  QString &filename = widget->getBannerName();

  // Create the task
  task = new getBannerTask(filename.toUtf8().data(), tvdb);

  // Connect the widget to the task
  QObject::connect(task, SIGNAL(bannerReady(tvdb_buffer_t*)), widget, SLOT(bannerReady(tvdb_buffer_t*)));
  QObject::connect(task, SIGNAL(bannerFailed()), widget, SLOT(bannerFailed()));

  // Add the task to the taskqueue
  tc.addTask(task);

  // Start the task
  tc.start();
}

void seasonEpisodeWidget::itemExpanded(QTreeWidgetItem *item)
{
  episodeInfoWidget *episode =NULL;
  QWidget *widget=NULL;
  QTreeWidgetItem *child=NULL;

  qDebug() << "Item text: " << item->text(0);

  if(item->childCount() == 1) {
    child = item->child(0);

    widget = ui->treeWidget->itemWidget(child, 0);

    // Check if the widget is an episodeInfoWidget.
    episode = dynamic_cast<episodeInfoWidget*>(widget);

    // When the picture is not downloaded, download it now.
    if(episode != NULL && episode->imageSet() == false){
      qDebug() << "Add Task to download banner.";
      addTask(episode);
    }
  }
}

QTreeWidgetItem *seasonEpisodeWidget::addSeasonEntry(int seasonNum)
{
  QString seasonName;
  QTreeWidgetItem *seasonItem=NULL;

  qDebug() << "Add season: " << seasonNum;

  // Build season name
  seasonName.sprintf("Season %d", seasonNum);

  // Create new season entry
  seasonItem = new QTreeWidgetItem();
  seasonName.sprintf("Season %d", seasonNum);
  seasonItem->setText(0, seasonName);
  ui->treeWidget->addTopLevelItem(seasonItem);

  return seasonItem;
}

void seasonEpisodeWidget::addEpisodeEntry(QTreeWidgetItem *seasonItem, tvdb_series_info_t *s)
{
  QString episodeStory;
  QString episodeName;
  QString bannerFilename;
  QTreeWidgetItem *episodeItem = NULL;
  QTreeWidgetItem *overviewItem = NULL;
  episodeInfoWidget *overviewWidget = NULL;

  // Add episode entry
  episodeItem = new QTreeWidgetItem(*seasonItem);
  episodeName.sprintf("%d - %s - %s", s->episode_number, s->episode_name, s->first_aired);
  episodeItem->setText(0, episodeName);
  seasonItem->addChild(episodeItem);
  overviewItem = new QTreeWidgetItem(episodeItem);
  //overviewItem->setText(0, s->overview);
  episodeItem->addChild(overviewItem);
  episodeStory = s->overview;
  bannerFilename = s->filename;
  overviewWidget = new episodeInfoWidget(s, overviewItem, ui->treeWidget);
  ui->treeWidget->setItemWidget(overviewItem, 0, overviewWidget);

  qDebug() << "	id [" << s->id << "], seriesid [" << s->series_id << "], name ["
      << s->episode_name << "] episode [" << s->episode_number << "]";
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
  QTreeWidgetItem *seasonItem = NULL;

  n = tvdb_list_next(seriesInfo);
  while(n != NULL) {
    s = (tvdb_series_info_t *)n->data;
    curSeason=s->season_number;
    curEpisode=s->episode_number;

    if(curSeason != 0 && curEpisode != 0){
      // Add season entry
      if(curSeason > widgetSeason)
      {
        seasonItem = addSeasonEntry(curSeason);

        // Set new season
        widgetSeason=curSeason;
      }
      if(seasonItem != NULL) {
        addEpisodeEntry(seasonItem, s);
      }
    }

    n = tvdb_list_next(seriesInfo);
  }
}