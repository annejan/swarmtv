#ifndef SEARCHCONTROL_HPP
#define SEARCHCONTROL_HPP

#include <QTableWidget>
#include <QWidget>

extern "C" {
#include <swarmtv.h>
}

#include "ui_mainwindow.h"

class searchControl : public QWidget
{
  Q_OBJECT
public:
  searchControl(QWidget *parent = 0);
  void setUi(Ui::MainWindow *ui);
  void initHeaders();

signals:

public slots:
  void searchClicked();
  void tableDownload(int row, int column);
  void showContextMenu(const QPoint&);

private:
  Ui::MainWindow *ui;
  void fillTable(newtorrents_container *newtorrents);
  int rowToId(int row);
};

#endif // SEARCHCONTROL_HPP
