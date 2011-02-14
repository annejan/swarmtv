#include "serieswidget.hpp"
//#include "singleton.h"
#include "taskinterface.hpp"
#include "taskqueue.hpp"
#include "getbannertask.hpp"
#include <iostream>

extern "C" {
#include <tvdb.h>
}

#include <QVBoxLayout>
#include <QLabel>
#include <QImage>

void seriesWidget::createLayout()
{
  //taskQueue *tq = &Singleton<taskQueue>::Instance();
  QVBoxLayout *layout = new QVBoxLayout(this);
  QHBoxLayout *hLayout = new QHBoxLayout();
  layout->setMargin(2);
  hLayout->setMargin(2);
  bannerImage = new QLabel();
  imageBuffer = (tvdb_buffer_t*) calloc(1, sizeof(tvdb_buffer_t));

  // Create thread to get Image from TheTvdb
  if(strlen(bannerName) != 0) {
    getBannerTask *banTask = new getBannerTask();
    banTask->setFilename(bannerName);
    QObject::connect(banTask, SIGNAL(bannerReady(tvdb_buffer_t*)),this, SLOT(imageReady(tvdb_buffer_t*)));
    QObject::connect(banTask, SIGNAL(bannerFailed()), this, SLOT(imageFailed()));
    tasks->addTask(banTask);
  }

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

seriesWidget::seriesWidget(tvdb_series_t *series, char *bannerName, taskQueue *tasks, QWidget *parent) :
    QWidget(parent)
{
  this->bannerName = bannerName;
  this->title = new QLabel(series->name);
  this->titleString = new QString(series->name);
  this->overview = new QLabel(series->overview);
  this->firstaired = new QLabel(series->first_aired);
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
