#ifndef EPISODEINFOWIDGET_HPP
#define EPISODEINFOWIDGET_HPP

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QString>
#include <QTreeWidget>

extern "C" {
  #include "tvdb.h"
}

class episodeInfoWidget : public QFrame
{
  Q_OBJECT
public:
  explicit episodeInfoWidget(QWidget *parent = 0);
  explicit episodeInfoWidget(tvdb_series_info_t *seriesInfo, QTreeWidgetItem *treeItem, QWidget *parent = 0);

  void setImageText(const QString &imageTest);
  void setStory(QString &story);
  void setTreeItem(QTreeWidgetItem &treeItem);

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
  QTreeWidgetItem *treeItem;
  QString bannerName;
};

#endif // EPISODEINFOWIDGET_HPP
