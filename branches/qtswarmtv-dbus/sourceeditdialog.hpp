#ifndef SOURCEEDITDIALOG_HPP
#define SOURCEEDITDIALOG_HPP

#include <QDialog>

namespace Ui {
    class sourceEditDialog;
}

class sourceEditDialog : public QDialog
{
    Q_OBJECT

public:
    //explicit sourceEditDialog(QWidget *parent = 0);
    sourceEditDialog(int id, QWidget *parent = 0);
    sourceEditDialog(QWidget *parent = 0);
    ~sourceEditDialog();
    void setNameEnable(bool enable);

public slots:
    void sourceAccepted();

private:
    int id;
    Ui::sourceEditDialog *ui;
    void fillDialog();
    void storeSource();
};

#endif // SOURCEEDITDIALOG_HPP
