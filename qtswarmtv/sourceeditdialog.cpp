#include <QWidget>

#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <swarmtv.h>

#include "sourceeditdialog.hpp"
#include "singleton.h"
#include "swarmtv.hpp"
#include "ui_sourceeditdialog.h"
#include "sourcetablecontrol.hpp"

sourceEditDialog::sourceEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sourceEditDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    // Connect signal
    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(sourceAccepted()));
}

sourceEditDialog::sourceEditDialog(int id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sourceEditDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);
    this->id = id;
    ui->setupUi(this);

    // Set values on the dialog
    fillDialog();

    // Connect signal
    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(sourceAccepted()));
}

sourceEditDialog::~sourceEditDialog()
{
    delete ui;
}

void sourceEditDialog::setNameEnable(bool enable)
{
    ui->nameSourceLineEdit->setEnabled(enable);
}

void sourceEditDialog::fillDialog()
{
    int rc=0;
    int index=0;
    source_struct *source=NULL;

    // Get swarmtv object
    swarmTv *swarm = &Singleton<swarmTv>::Instance();

    // Get source information
    rc = rsstgetsource(swarm->getHandle(), id, &source);

    // Set data to gui
    ui->nameSourceLineEdit->setText(source->name);
    ui->urlSourceLineEdit->setText(source->url);
    index = ui->metaComboBox->findText(source->metatype);
    ui->metaComboBox->setCurrentIndex(index);
    index = ui->ParserComboBox->findText(source->parser);
    ui->ParserComboBox->setCurrentIndex(index);

    // Free source struct
    rsstfreesource(source);
}

void sourceEditDialog::storeSource()
{
    source_struct source;

    // Get swarmtv class
    swarmTv *swarm = &Singleton<swarmTv>::Instance();

    // Get values from the dialog
    source.id = 0; // Just to make things tidy
    source.name =  strdup(ui->nameSourceLineEdit->displayText().toUtf8().data());
    source.url =  strdup(ui->urlSourceLineEdit->text().toUtf8().data());
    source.parser =  strdup(ui->ParserComboBox->currentText().toUtf8().data());
    source.metatype =  strdup(ui->metaComboBox->currentText().toUtf8().data());

    // Store the rss source into swarmtv
    rsstaddsource(swarm->getHandle(), &source);

        // Free string in struct
        rsstfreesource(&source);
}

void sourceEditDialog::sourceAccepted()
{
    sourceTableControl *ctrl;

    // Safe filter
    this->storeSource();

    // Update Dialog
    ctrl =  static_cast<sourceTableControl*>(this->parent());
    ctrl->updateTable();

    // Close Dialog
    this->accept();
}
