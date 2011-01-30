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

    void setName(QString *name);
    void setTitle(QString *title);

    void setMaxSize(size_t max);
    void setMinSize(size_t min);
    void setMaxSize(QString *max);
    void setMinSize(QString *min);

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
