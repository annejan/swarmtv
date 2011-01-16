#include <sqlite3.h>
#include <stdlib.h>

extern "C" {
#include <swarmtv.h>
}

#include <iostream>

#include "swarmtv.hpp"
#include "simpleeditdialog.hpp"
#include "simpletablecontrol.hpp"
#include "ui_simpleeditdialog.h"
#include "readablesize.hpp"
#include "singleton.h"
#include "testfilterdialog.hpp"

simpleEditDialog::simpleEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::simpleEditDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    // Set mode to add
    mode = add;

    // Connect signal
    QObject::connect(ui->simpleButtonBox, SIGNAL(accepted()), this, SLOT(simpleAccepted()));
}


simpleEditDialog::simpleEditDialog(int id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::simpleEditDialog)
{
  setAttribute(Qt::WA_DeleteOnClose);

  this->id=id;
  mode = edit;

  // Setup the gui widgets
  ui->setupUi(this);

  // Set values on the dialog
  fillDialog();

  // Connect signal
  QObject::connect(ui->simpleButtonBox, SIGNAL(accepted()), this, SLOT(simpleAccepted()));
  QObject::connect(ui->testPushButton, SIGNAL(clicked()), this, SLOT(testClicked()));
  QObject::connect(ui->guessSEPushButton, SIGNAL(clicked()), this, SLOT(fillSeasonEpisode()));
}


simpleEditDialog::~simpleEditDialog()
{
  delete ui;
}



void simpleEditDialog::enableNameEnable(bool enable)
{
  ui->nameSimpleLineEdit->setEnabled(enable);
}

void simpleEditDialog::testClicked()
{
  simplefilter_struct simple;

  // Clear simple struct
  memset(&simple, 0, sizeof(simplefilter_struct));

  // Get filter object
  getFromUi(simple);

  // Create Object
  new testFilterDialog(simple);
}

void simpleEditDialog::simpleAccepted()
{
  simplefilter_struct simple;
  simpleTableControl *stc=NULL;

  // Get SwarmTv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Get all data from the gui into a simple_struct
  getFromUi(simple);

  // Store the simple struct
  if(mode == edit) {
    rssteditsimplefilter(swarm->getHandle(), &simple);
  } else {
    rsstaddsimplefilter(swarm->getHandle(), &simple);
  }

  // Free allocated structure
  rsstfreesimplefilter(&simple);

  // Update parent
  stc = (simpleTableControl*) this->parent();
  stc->updateTable();

  // Close dialog
  this->close();
}


void simpleEditDialog::fillDialog()
{
  simplefilter_struct *simple=NULL;
  readableSize sizeconv;
  std::string size;
  int index=0;

  // Get SwarmTv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Get data from swarmtv
  rsstgetsimplefilterid(swarm->getHandle(), &simple, id);

  // Put the data into the different widgets in the dialog
  ui->nameSimpleLineEdit->setText(simple->name);
  ui->titleSimpleLineEdit->setText(simple->title);
  ui->excludeSimpleLineEdit->setText(simple->exclude);
  ui->categorySimpleLineEdit->setText(simple->category);
  ui->sourceSimpleLineEdit->setText(simple->source);

  sizeconv.setSize(simple->minsize);
  sizeconv.getSize(size);
  ui->minSizeSimpleLineEdit->setText(size.c_str());
  sizeconv.setSize(simple->maxsize);
  sizeconv.getSize(size);
  ui->maxSizeSimpleLineEdit->setText(size.c_str());

  ui->seasonSpinBox->setValue(simple->fromseason);
  ui->episodeSpinBox->setValue(simple->fromepisode);

  index = ui->nodupSimpleComboBox->findText(simple->nodup);
  ui->nodupSimpleComboBox->setCurrentIndex(index);

  // Free structure
  rsstfreesimplefilter(simple);
  free(simple);
}


void simpleEditDialog::getFromUi(simplefilter_struct &simple)
{
  char *value=NULL;
  readableSize sizeconv;
  size_t uisize;
  std::string uiSizeStr;

  // Get plain text fields
  simple.id=id;
  value = strdup(ui->nameSimpleLineEdit->text().toUtf8().data());
  simple.name = value;
  value = strdup(ui->titleSimpleLineEdit->text().toUtf8().data());
  simple.title = value;
  value = strdup(ui->excludeSimpleLineEdit->text().toUtf8().data());
  simple.exclude = value;
  value = strdup(ui->categorySimpleLineEdit->text().toUtf8().data());
  simple.category = value;
  value = strdup(ui->sourceSimpleLineEdit->text().toUtf8().data());
  simple.source = value;
  value = strdup(ui->nodupSimpleComboBox->currentText().toAscii().data());
  simple.nodup = value;

  // Convert sizes back to size_t
  uiSizeStr = ui->minSizeSimpleLineEdit->text().toAscii().data();
  sizeconv.setSize(uiSizeStr);
  sizeconv.getSize(uisize);
  simple.minsize=uisize;
  uiSizeStr = ui->maxSizeSimpleLineEdit->text().toAscii().data();
  sizeconv.setSize(uiSizeStr);
  sizeconv.getSize(uisize);
  simple.maxsize=uisize;

  // Get from season/episode
  simple.fromseason = ui->seasonSpinBox->value();
  simple.fromepisode = ui->episodeSpinBox->value();
}


void simpleEditDialog::fillSeasonEpisode()
{
  int rc=0;
  simplefilter_struct filter;
  int season=0;
  int episode=0;

  // Get SwarmTv handle
  swarmTv *swarm = &Singleton<swarmTv>::Instance();

  // Get values from interface and build the filter object
  this->getFromUi(filter);

  // Get season and episode from number using current filter setup
  rc = rsstgetnewestepisode(&filter, &season, &episode);

  // Fill out fields in gui
  ui->seasonSpinBox->setValue(season);
  ui->episodeSpinBox->setValue(episode);

  // Free Simple filter
  rsstfreesimplefilter(&filter);
}
