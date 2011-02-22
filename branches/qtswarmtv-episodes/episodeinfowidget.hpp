#ifndef EPISODEINFOWIDGET_HPP
#define EPISODEINFOWIDGET_HPP

#include <QWidget>
#include <QLabel>
#include <QString>

#include "tvdb.h"

class episodeInfoWidget : public QWidget
{
  Q_OBJECT
public:
  explicit episodeInfoWidget(QWidget *parent = 0);
  explicit episodeInfoWidget(QString &storyName, QString &bannerName, QWidget *parent = 0);

  void setImageText(const QString &imageTest);
  void setStory(QString &story);

  QString &getBannerName();
  bool 		imageSet();

signals:

public slots:
  void bannerReady(tvdb_buffer_t *tvdb);
  void bannerFailed();

private:
  void init();

  QLabel *bannerImage;
  QLabel *story;
  QString bannerName;
};

#endif // EPISODEINFOWIDGET_HPP
