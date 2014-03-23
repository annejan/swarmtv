#include "serieswidget.hpp"
//#include "singleton.h"
#include "taskinterface.hpp"
#include "taskqueue.hpp"
#include "getbannertask.hpp"
#include <iostream>
#include <QTextCodec>

extern "C" {
#include <tvdb.h>
}

#include <QVBoxLayout>
#include <QLabel>
#include <QImage>

// Add task to the queue in order to retrieve banner image.
void seriesWidget::addBannerTask(const QString &bannerString)
{
  getBannerTask *banTask=NULL;

  if(bannerString.length() != 0){
    banTask = new getBannerTask();
    banTask->setFilename(bannerString.toUtf8().data());
    QObject::connect(banTask, SIGNAL(bannerReady(tvdb_buffer_t*)),this, SLOT(imageReady(tvdb_buffer_t*)));
    QObject::connect(banTask, SIGNAL(bannerFailed()), this, SLOT(imageFailed()));
    tasks->addTask(banTask);
  }
}

void seriesWidget::createLayout()
{
  //taskQueue *tq = &Singleton<taskQueue>::Instance();
  QVBoxLayout *layout = new QVBoxLayout(this);
  QHBoxLayout *hLayout = new QHBoxLayout();
  layout->setMargin(2);
  hLayout->setMargin(2);
  bannerImage = new QLabel();
  imageBuffer = (tvdb_buffer_t*) calloc(1, sizeof(tvdb_buffer_t));

  addBannerTask(QString(bannerName));
#if 0
  // Create thread to get Image from TheTvdb
  if(strlen(bannerName) != 0) {
    getBannerTask *banTask = new getBannerTask();
    banTask->setFilename(bannerName);
    QObject::connect(banTask, SIGNAL(bannerReady(tvdb_buffer_t*)),this, SLOT(imageReady(tvdb_buffer_t*)));
    QObject::connect(banTask, SIGNAL(bannerFailed()), this, SLOT(imageFailed()));
    tasks->addTask(banTask);
  }
#endif

  // Set layout
  title->setFont(QFont("Times",20,QFont::Bold));
  overview->setWordWrap(true);

  // Set title and Rating to the hLayout
  hLayout->addWidget(title);
  hLayout->addWidget(firstaired);

  // Add labels to layout
  layout->addWidget(bannerImage);
  layout->addLayout(hLayout);
  layout->addWidget(overview);
}

#if 0
seriesWidget::seriesWidget(QWidget *parent) :
    QWidget(parent)
{
  this->bannerName = NULL;
  this->title = new QLabel("");
  this->overview = new QLabel("");

  createLayout();
}
#endif

seriesWidget::seriesWidget(tvdb_series_t *series, char *bannerName, taskQueue *tasks, int seriesId, QWidget *parent) :
    QWidget(parent)
{
  QTextCodec *codec = QTextCodec::codecForName("utf-8");
  QString converted;

  this->bannerName = bannerName;

  // Process name
  converted = codec->toUnicode(series->name);
  this->title = new QLabel(converted);
  this->titleString = new QString(converted);

  // Process overview
  converted = codec->toUnicode(series->overview);
  this->overview = new QLabel(converted);

  // Handle rest
  this->firstaired = new QLabel(series->first_aired);
  this->seriesId = seriesId;
  this->tasks = tasks;

  createLayout();
}

seriesWidget::~seriesWidget()
{
  delete(titleString);
  delete(title);
  delete(overview);
}

void seriesWidget::imageReady(tvdb_buffer_t *banner)
{
  QPixmap *bannerPixmap = new QPixmap(); // Image to show as banner

  // set banner information to the Image
  bannerPixmap->loadFromData((uchar*) banner->memory, (int) banner->size);
  bannerImage->setPixmap(*bannerPixmap);
}

void seriesWidget::imageFailed(void)
{
  // Set bogus image in future
}

QString *seriesWidget::getTitle()
{
  return titleString;
}

int seriesWidget::getSeriesId()
{
  return seriesId;
}

