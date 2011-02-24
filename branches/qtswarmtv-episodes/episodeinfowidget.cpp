#include "episodeinfowidget.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include <QDebug>
#include <QTreeView>
#include <QSize>

episodeInfoWidget::episodeInfoWidget(QWidget *parent) :
    QWidget(parent)
{
  init();
}

episodeInfoWidget::episodeInfoWidget(QString &storyName, QString &bannerName,
                                     QTreeWidgetItem *treeItem, QWidget *parent) :
    QWidget(parent)
{
  init();
  story->setText(storyName);
  this->treeItem = treeItem;
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
  QTreeView *view = dynamic_cast<QTreeView*>(parent());
  int oldHeight=0;

  // set banner information to the Image
  bannerPixmap->loadFromData((uchar*) tvdb->memory, (int) tvdb->size);
  bannerImage->setPixmap(*bannerPixmap);
  qDebug() << "Banner ready called.";
  oldHeight=this->height();

  // Try to adjust the widget size
  this->setMinimumHeight(oldHeight + bannerPixmap->height());
  QSize itemSize = treeItem->sizeHint(0);
  itemSize.setHeight(oldHeight + bannerPixmap->height());
  treeItem->setSizeHint(0, itemSize);
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
