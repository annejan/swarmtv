#include "episodeinfowidget.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QDebug>
#include <QTreeView>
#include <QSize>
#include <QFrame>


episodeInfoWidget::episodeInfoWidget(QWidget *parent) :
    QFrame(parent)
{
  init();
}

episodeInfoWidget::episodeInfoWidget(QString &storyName, QString &bannerName,
                                     QTreeWidgetItem *treeItem, QWidget *parent) :
    QFrame(parent)
{
  init();
  story->setText(storyName);
  this->treeItem = treeItem;
  this->bannerName = bannerName;
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
  //QTreeView *view = dynamic_cast<QTreeView*>(parent());
  //int oldHeight=0;

  // set banner information to the Image
  bannerPixmap->loadFromData((uchar*) tvdb->memory, (int) tvdb->size);
  if(bannerPixmap->width() != 0){
    bannerImage->setPixmap(*bannerPixmap);
  } else {
    bannerImage->setText(tr("No Image available."));
  }
  //oldHeight=this->height();

  // Try to adjust the widget size
  //this->setMinimumHeight(oldHeight + bannerPixmap->height());
  //QSize itemSize = treeItem->sizeHint(0);
  //itemSize.setHeight(oldHeight + bannerPixmap->height());
  //treeItem->setSizeHint(0, itemSize);
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
