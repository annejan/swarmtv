#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <QSettings>

extern "C" {
#include <swarmtv.h>
}

#include <QMessageBox>

#include "swarmtv.hpp"
#include "singleton.h"
#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"

static const QString TVDB_API_CONFIG("config/tvdbapiconfig");

settingsDialog::settingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingsDialog)
{
    ui->setupUi(this);
}

settingsDialog::~settingsDialog()
{
    delete ui;
}

void settingsDialog::show()
{
  int rc=0;
  config_container *configitems=NULL;
  QSettings settings;

  // Get data from SwarmTv
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Set data in widgets
  rc = rsstgetallconfig(swarm->getHandle(), &configitems);
  if(rc != 0){
    QMessageBox::warning(this, tr("SwarmTv"),
                         tr("Could not retrieve settings from SwarmTv."),
                         QMessageBox::Ok);
    // Exit
    return;
  }

  // Set retrieved values into setup
  ui->refreshLineEdit->setText("900");
  this->containerToUI(configitems);
  ui->apiKeyLineEdit->setText(settings.value(TVDB_API_CONFIG).toString());

  // Free the settings container
  rsstfreeconfigcontainer(configitems);

  QWidget::show();
}

void settingsDialog::containerToUI(config_container *configitems)
{
  int count=0;

  // The names of the config settings we are interested in
  std::string torrentdir ("torrentdir");
  std::string nzbdir		 ("nzbdir");
  std::string refresh		 ("refresh");
  std::string retain		 ("retain");
  std::string minsize		 ("min_size");
  std::string smtp_enable("smtp_enable");
  std::string smtp_to		 ("smtp_to");
  std::string smtp_from	 ("smtp_from");
  std::string smtp_host	 ("smtp_host");
  std::string choice_true ("Y");

  while(count < configitems->nr){
    if(torrentdir.compare(configitems->config[count].name) == 0){
      ui->torrentDirLineEdit->setText(configitems->config[count].value);
    }
    else if(nzbdir.compare(configitems->config[count].name) == 0){
      ui->nzbDirectoryLineEdit->setText(configitems->config[count].value);
    }
    else if(refresh.compare(configitems->config[count].name) == 0){
      ui->refreshLineEdit->setText(configitems->config[count].value);
    }
    else if(retain.compare(configitems->config[count].name) == 0){
      ui->retainLineEdit->setText(configitems->config[count].value);
    }
    else if(minsize.compare(configitems->config[count].name) == 0){
      ui->minimalSizeLineEdit->setText(configitems->config[count].value);
    }
    else if(smtp_enable.compare(configitems->config[count].name) == 0){
      if(choice_true.compare(configitems->config[count].value) == 0){
        ui->emailEnableCheckBox->setChecked(true);
      } else {
        ui->emailEnableCheckBox->setChecked(false);
      }
    }
    else if(smtp_to.compare(configitems->config[count].name) == 0){
      ui->destinationLineEdit->setText(configitems->config[count].value);
    }
    else if(smtp_from.compare(configitems->config[count].name) == 0){
      ui->senderLineEdit->setText(configitems->config[count].value);
    }
    else if(smtp_host.compare(configitems->config[count].name) == 0){
      ui->ServerLineEdit->setText(configitems->config[count].value);
    }

    count++;
  }
}

void settingsDialog::accept()
{
  QSettings settings;

  // Get the SwarmTv pointer
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Store the set values
  // The names of the config settings we are interested in
  std::string torrentdir ("torrentdir");
  std::string nzbdir		 ("nzbdir");
  std::string refresh		 ("refresh");
  std::string retain		 ("retain");
  std::string minsize		 ("min_size");
  std::string smtp_enable("smtp_enable");
  std::string smtp_to		 ("smtp_to");
  std::string smtp_from	 ("smtp_from");
  std::string smtp_host	 ("smtp_host");
  std::string choice_true ("Y");
  std::string choice_false ("N");

  // Store values
  rsstsetconfigitem(swarm->getHandle(), torrentdir.c_str(), ui->torrentDirLineEdit->text().toUtf8().data());
  rsstsetconfigitem(swarm->getHandle(), nzbdir.c_str(), ui->nzbDirectoryLineEdit->text().toUtf8().data());
  rsstsetconfigitem(swarm->getHandle(), refresh.c_str(), ui->refreshLineEdit->text().toUtf8().data());
  rsstsetconfigitem(swarm->getHandle(), retain.c_str(), ui->retainLineEdit->text().toUtf8().data());
  rsstsetconfigitem(swarm->getHandle(), minsize.c_str(), ui->minimalSizeLineEdit->text().toUtf8().data());
  rsstsetconfigitem(swarm->getHandle(), smtp_to.c_str(), ui->destinationLineEdit->text().toUtf8().data());
  rsstsetconfigitem(swarm->getHandle(), smtp_from.c_str(), ui->senderLineEdit->text().toUtf8().data());
  rsstsetconfigitem(swarm->getHandle(), smtp_host.c_str(), ui->ServerLineEdit->text().toUtf8().data());

  // Store values in the config settings
  settings.setValue(TVDB_API_CONFIG, ui->apiKeyLineEdit->text());

  if(ui->emailEnableCheckBox->checkState() == Qt::Checked) {
    rsstsetconfigitem(swarm->getHandle(), smtp_enable.c_str(), choice_true.c_str());
  } else {
    rsstsetconfigitem(swarm->getHandle(), smtp_enable.c_str(), choice_false.c_str());
  }

  QDialog::accept();
}
