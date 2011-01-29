#include "serieswidget.hpp"
#include <iostream>

#include <QVBoxLayout>
#include <QLabel>
#include <QImage>

void seriesWidget::createLayout()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(2);
  QPixmap *bannerPixmap = new QPixmap(); // Image to show as banner
  bannerImage = new QLabel();

  std::cout << "Added: " << title->text().toUtf8().begin() << std::endl;

  // Create labels
  //QLabel *titleLabel = new QLabel();
  //QLabel *overviewLabel = new QLabel();

  // Set Content
  //titleLabel->setText(title->text());
  //overviewLabel->setText(overview->text());
  if(banner->size != 0) {
    bannerPixmap->loadFromData((uchar*) banner->memory, (int) banner->size);
    bannerImage->setPixmap(*bannerPixmap);
  }

  // Set layout
  title->setFont(QFont("Times",20,QFont::Bold));
  overview->setWordWrap(true);


  // Add labels to layout
  if(banner->size != 0) {
    layout->addWidget(bannerImage);
  }
  layout->addWidget(title);
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

seriesWidget::seriesWidget(QString &title, QString &overview, tvdb_buffer_t *banner,  QWidget *parent)
{
  this->banner = banner;
  this->title = new QLabel(title);
  this->overview = new QLabel(overview);

  createLayout();
}

seriesWidget::~seriesWidget()
{
  delete(title);
  delete(overview);
}
