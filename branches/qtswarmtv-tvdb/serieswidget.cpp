#include "serieswidget.hpp"
#include <iostream>

#include <QVBoxLayout>
#include <QLabel>
#include <QImage>

void seriesWidget::createLayout()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  QHBoxLayout *hLayout = new QHBoxLayout();
  layout->setMargin(2);
  hLayout->setMargin(2);
  QPixmap *bannerPixmap = new QPixmap(); // Image to show as banner
  bannerImage = new QLabel();

  std::cout << "Added: " << title->text().toUtf8().begin() << std::endl;

  // Set Content
  if(banner->size != 0) {
    bannerPixmap->loadFromData((uchar*) banner->memory, (int) banner->size);
    bannerImage->setPixmap(*bannerPixmap);
  }

  // Set layout
  title->setFont(QFont("Times",20,QFont::Bold));
  overview->setWordWrap(true);

  // Set title and Rating to the hLayout
  hLayout->addWidget(title);
  hLayout->addWidget(firstaired);

  // Add labels to layout
  if(banner->size != 0) {
    layout->addWidget(bannerImage);
  }
  layout->addLayout(hLayout);
  layout->addWidget(overview);
}

seriesWidget::seriesWidget(QWidget *parent) :
    QWidget(parent)
{
  this->banner = NULL;
  this->title = new QLabel("");
  this->overview = new QLabel("");

  createLayout();
}

seriesWidget::seriesWidget(tvdb_series_t *series, tvdb_buffer_t *banner,  QWidget *parent)
{
  this->banner = banner;
  this->title = new QLabel(series->name);
  this->overview = new QLabel(series->overview);
  this->firstaired = new QLabel(series->first_aired);

  createLayout();
}

seriesWidget::~seriesWidget()
{
  delete(title);
  delete(overview);
}

QString *seriesWidget::getTitle()
{
  return &(title->text());
}
