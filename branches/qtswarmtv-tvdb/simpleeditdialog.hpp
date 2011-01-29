#ifndef SIMPLEEDITDIALOG_HPP
#define SIMPLEEDITDIALOG_HPP

#include <QDialog>

extern "C" {
#include <swarmtv.h>
}

namespace Ui {
    class simpleEditDialog;
}

class simpleEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit simpleEditDialog(QWidget *parent = 0);
    simpleEditDialog(int id, QWidget *parent = 0);
    ~simpleEditDialog();

    void enableNameEnable(bool enable);
    void getFromUi(simplefilter_struct &simple);
    typedef enum{add, edit} addoredit;

public slots:
    void simpleAccepted();
    void testClicked();
    void fillSeasonEpisode();

private:
    addoredit mode;
    int id;
    void fillDialog();
    Ui::simpleEditDialog *ui;
};

#endif // SIMPLEEDITDIALOG_HPP
