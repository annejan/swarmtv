#ifndef DOWNLOADEDTABLECONTROL_HPP
#define DOWNLOADEDTABLECONTROL_HPP

#include <QTableWidget>

#include <QWidget>

class downloadedTableControl : public QWidget
{
  Q_OBJECT
public:
  explicit downloadedTableControl(QWidget *parent = 0);

  void setTable(QTableWidget *downtable);

  void fillTable();

signals:

public slots:
  void delClicked();
  void cellDoubleClicked(int,int);

private:
  void initHeaders();
  QTableWidget *downtable;
  int getRowSelected();
};

#endif // DOWNLOADEDTABLECONTROL_HPP
