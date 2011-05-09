#include "seasonepisodewidget.hpp"
#include "ui_seasonepisodewidget.h"

extern "C" {
  #include "tvdb.h"
}

#include <QSettings>
#include <QDebug>
#include <QTreeWidget>
#include <QTextCodec>

#include "episodeinfowidget.hpp"
#include "getbannertask.hpp"
#include "getseriesinfotask.hpp"

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
  QObject::connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));
}

void seasonEpisodeWidget::setSeriesId(int id)
{
  seriesId=id;
}

void seasonEpisodeWidget::retrieveEpisodeData()
{

  // Disable the tree view to show we are searching
  ui->treeWidget->setDisabled(true);

  // Create task object
  getSeriesInfoTask *search = new getSeriesInfoTask(tvdb, seriesId, this);

  // connect signal
  QObject::connect(search, SIGNAL(results(tvdb_buffer_t*)), this, SLOT(seriesResults(tvdb_buffer_t*)));
  QObject::connect(search, SIGNAL(failed()), this, SLOT(seriesFailed()));

  // Add task queue
  tc.addTask(search);

  // start taskqueue
  tc.start();
}

void seasonEpisodeWidget::seriesResults(tvdb_buffer_t *series_xml)
{
  int rc=0;
  tvdb_list_front_t seriesInfo;

  // Initialize structs
  memset(&seriesInfo, 0, sizeof(tvdb_list_front_t));

  // Parse the XML data
  rc = tvdb_parse_series_info(series_xml, "", &seriesInfo);
  if(rc != TVDB_OK){
    qDebug() << "tvdb_parse_series_info returned not okay.";
  }

  // Add the data to the ListView
  fillListView(&seriesInfo);

  // Detroy the tvdb instance
  tvdb_list_remove(&seriesInfo);

  // All is done, enable again
  ui->treeWidget->setEnabled(true);
}

void seasonEpisodeWidget::seriesFailed()
{
  qDebug() << "Downloading of series info failed.";
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

  if(item->childCount() == 1) {
    child = item->child(0);

    widget = ui->treeWidget->itemWidget(child, 0);

    // Check if the widget is an episodeInfoWidget.
    episode = dynamic_cast<episodeInfoWidget*>(widget);

    // When the picture is not downloaded, download it now.
    if(episode != NULL && episode->imageSet() == false){
      addTask(episode);
    }
  }
}

void seasonEpisodeWidget::itemDoubleClicked(QTreeWidgetItem *item, int column)
{
  episodeInfoWidget *episode =NULL;
  QWidget *widget=NULL;

  // make sure the click is valid.
  if(column != 0) {
    return;
  }

  // Get the item pointer.
  widget = ui->treeWidget->itemWidget(item, 0);
  episode = dynamic_cast<episodeInfoWidget*>(widget);

  // When a widget handle episode, when season handle season.
  if(episode != NULL){
    // print episode title and season/episode
    qDebug() << "Double clicked on series: " << episode->getSeriesName() << " S" << episode->getSeason() << "E" << episode->getEpisode();
  } else {
    // print series number and name
    qDebug() << "Double clicked season";
  }
}

QTreeWidgetItem *seasonEpisodeWidget::addSeasonEntry(int seasonNum)
{
  QString seasonName;
  QTreeWidgetItem *seasonItem=NULL;

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
  episodeItem->addChild(overviewItem);
  episodeStory = s->overview;
  bannerFilename = s->filename;
  overviewWidget = new episodeInfoWidget(s, overviewItem, seriesName, ui->treeWidget);
  ui->treeWidget->setItemWidget(overviewItem, 0, overviewWidget);
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
