#ifndef SETTINGSDIALOG_HPP
#define SETTINGSDIALOG_HPP
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

extern "C" {
#include <swarmtv.h>
}

#include <QDialog>

namespace Ui {
    class settingsDialog;
}

class settingsDialog : public QDialog
{
    Q_OBJECT

public:
  explicit settingsDialog(QWidget *parent = 0);
  ~settingsDialog();

public slots:
  void show();
  void accept();

private:
  Ui::settingsDialog *ui;
  void containerToUI(config_container *configitems);
};

#endif // SETTINGSDIALOG_HPP
