#ifndef NEWTORRENTFULLINFODIALOG_HPP
#define NEWTORRENTFULLINFODIALOG_HPP

#include <QDialog>

namespace Ui {
    class newTorrentFullInfoDialog;
}

class newTorrentFullInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit newTorrentFullInfoDialog(int newtorid, QWidget *parent = 0);
    ~newTorrentFullInfoDialog();


private:
    void fillDialog();

    Ui::newTorrentFullInfoDialog *ui;
    int newtorid;
};

#endif // NEWTORRENTFULLINFODIALOG_HPP
