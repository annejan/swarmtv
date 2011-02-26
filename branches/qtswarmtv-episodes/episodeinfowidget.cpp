#include "episodeinfowidget.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QDebug>
#include <QTreeView>
#include <QSize>
#include <QFrame>
#include <QTextCodec>

episodeInfoWidget::episodeInfoWidget(QWidget *parent) :
    QFrame(parent)
{
  init();
}

episodeInfoWidget::episodeInfoWidget(tvdb_series_info_t *seriesInfo, QTreeWidgetItem *treeItem, QWidget *parent) :
    QFrame(parent)
{
  QTextCodec *codec = QTextCodec::codecForName("utf-8");
  QString overview;

  init();

  // Convert and set overview
  overview = codec->toUnicode(seriesInfo->overview);
  story->setText(overview);

  this->bannerName = seriesInfo->filename;
  this->treeItem = treeItem;
}

void episodeInfoWidget::init()
{
  // Configure containing frame
  this->setFrameStyle(QFrame::Box);
  this->setMinimumHeight(250);

  // Create layout inside QFrame
  QHBoxLayout *layer = new QHBoxLayout(this);

  // Insert and initialize the image-label
  bannerImage = new QLabel("<font color='grey'>Downloading Image</font>");
  bannerImage->setFrameStyle(QFrame::Box);
  bannerImage->setMaximumWidth(320);
  bannerImage->setMaximumHeight(240);
  bannerImage->setAlignment(Qt::AlignCenter);

  // Set the episode story/spoiler
  story = new QLabel("");
  story->setWordWrap(true);

  // Add widgets to layout.
  layer->addWidget(bannerImage);
  layer->addWidget(story);
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

  // set banner information to the Image
  bannerPixmap->loadFromData((uchar*) tvdb->memory, (int) tvdb->size);
  if(bannerPixmap->width() != 0){
    bannerImage->setPixmap(*bannerPixmap);
  } else {
    bannerImage->setText(tr("No Image available."));
  }
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

void episodeInfoWidget::setTreeItem(QTreeWidgetItem &treeItem)
{
  this->treeItem=&treeItem;
}
