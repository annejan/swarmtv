#include "episodeinfowidget.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include <QDebug>

episodeInfoWidget::episodeInfoWidget(QWidget *parent) :
    QWidget(parent)
{
  init();
}

episodeInfoWidget::episodeInfoWidget(QString &storyName, QString &bannerName, QWidget *parent) :
    QWidget(parent)
{
  init();
  story->setText(storyName);
  this->bannerName = bannerName;
}

void episodeInfoWidget::init()
{
  QVBoxLayout *layer = new QVBoxLayout(this);
  bannerImage = new QLabel("");
  story = new QLabel("");
  layer->addWidget(bannerImage);
  layer->addWidget(story);

  // set label characteristics
  story->setWordWrap(true);
}

void episodeInfoWidget::setStory(QString &story)
{
  this->story->setText(story);
}

void episodeInfoWidget::setImageText(const QString &imageTest)
{
  bannerImage->setText(imageTest);
}

void episodeInfoWidget::bannerReady(tvdb_buffer_t *tvdb)
{
  QPixmap *bannerPixmap = new QPixmap(); // Image to show as banner
  int oldHeight=0;

  // set banner information to the Image
  bannerPixmap->loadFromData((uchar*) tvdb->memory, (int) tvdb->size);
  bannerImage->setPixmap(*bannerPixmap);
  qDebug() << "Banner ready called.";
  oldHeight=this->height();
  this->setMinimumHeight(oldHeight + bannerPixmap->height());
}

void episodeInfoWidget::bannerFailed()
{
  bannerImage->setText(tr("No banner"));
}

QString &episodeInfoWidget::getBannerName()
{
  return bannerName;
}

bool episodeInfoWidget::imageSet()
{
  if(bannerImage->pixmap() == NULL) {
    return false;
  } else {
    return true;
  }
}
