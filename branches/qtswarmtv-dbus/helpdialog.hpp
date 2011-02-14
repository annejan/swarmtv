#ifndef HELPDIALOG_HPP
#define HELPDIALOG_HPP

#include <QDialog>

namespace Ui {
    class helpDialog;
}

class helpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit helpDialog(QWidget *parent = 0);
    ~helpDialog();

private:
    Ui::helpDialog *ui;
};

#endif // HELPDIALOG_HPP
