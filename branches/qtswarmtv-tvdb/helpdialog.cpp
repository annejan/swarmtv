#include "helpdialog.hpp"
#include "ui_helpdialog.h"

helpDialog::helpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::helpDialog)
{
    ui->setupUi(this);
    // open links in 'real' browser
    ui->about->setOpenExternalLinks(true);
    ui->license->setOpenExternalLinks(true);
}

helpDialog::~helpDialog()
{
    delete ui;
}
