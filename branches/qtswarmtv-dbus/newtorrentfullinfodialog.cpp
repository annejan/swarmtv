#include <sqlite3.h>
#include <stdlib.h>

extern "C" {
#include <swarmtv.h>
}

#include <iostream>

#include "swarmtv.hpp"
#include "singleton.h"
#include "readablesize.hpp"
#include "newtorrentfullinfodialog.hpp"
#include "ui_newtorrentfullinfodialog.h"

newTorrentFullInfoDialog::newTorrentFullInfoDialog(int newtorid, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::newTorrentFullInfoDialog)
{
    ui->setupUi(this);
    this->newtorid = newtorid;

    // Set delete on close()
    setAttribute(Qt::WA_DeleteOnClose);

    // Fill the Dialog
    fillDialog();
}

newTorrentFullInfoDialog::~newTorrentFullInfoDialog()
{
    delete ui;
}

void newTorrentFullInfoDialog::fillDialog()
{
  int rc=0;
  newtorrents_struct newtor;
  readableSize readable;
  std::string human;

  // Get Swarmtv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Get object from SwarmTv
  rc = rsstnewtorrentsbyid(swarm->getHandle(), this->newtorid, &newtor);
  if(rc == 0) {
    // Put value into the dialog fields.
    ui->titleLineEdit->setText(newtor.title);
    ui->LinkLineEdit->setText(newtor.link);
    ui->pubDateLineEdit->setText(ctime(&(newtor.pubdate)));
    ui->categoryLineEdit->setText(newtor.category);
    ui->seasonSpinBox->setValue(newtor.season);
    ui->episodeSpinBox->setValue(newtor.episode);
    ui->seedsSpinBox->setValue(newtor.seeds);
    ui->peersSpinBox->setValue(newtor.peers);
    readable.setSize(newtor.size);
    readable.getSize(human);
    ui->sizeLineEdit->setText(human.c_str());
    ui->sourceLineEdit->setText(newtor.source);
    ui->metatypeLineEdit->setText(newtor.metatype);
  }

  // Free object
  rsstfreenewtor(&newtor);

}
