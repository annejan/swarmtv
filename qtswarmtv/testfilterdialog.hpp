#ifndef TESTFILTERDIALOG_HPP
#define TESTFILTERDIALOG_HPP

#include <QDialog>

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

extern "C" {
#include <swarmtv.h>
}

namespace Ui {
    class testFilterDialog;
}

class testFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit testFilterDialog(simplefilter_struct &simple, QWidget *parent = 0);
    ~testFilterDialog();

private:
    Ui::testFilterDialog *ui;
    void handleTest(simplefilter_struct &simple);
    void fillTable(newtorrents_container *newtorrents);
    void initHeaders();
};

#endif // TESTFILTERDIALOG_HPP
