#include "mrtShutterGui.h"
#include "ui_mrtShutterGui.h"

MrtShutterGui::MrtShutterGui(QWidget *parent) :
  ComponentGui(new MrtShutter(parent), true, parent),
  ui(new Ui::MrtShutterGui)
{
  init();
}

MrtShutterGui::MrtShutterGui(MrtShutter *sht, QWidget *parent) :
  ComponentGui(sht, false, parent),
  ui(new Ui::MrtShutterGui)
{
  init();
}


MrtShutterGui::~MrtShutterGui() {
  delete ui;
}

void MrtShutterGui::init() {

  ui->setupUi(this);

  connect(ui->cycle, SIGNAL(valueEdited(int)), component(), SLOT(setCycle(int)));
  connect(ui->exposure, SIGNAL(valueEdited(int)), component(), SLOT(setExposure(int)));
  connect(ui->repititions, SIGNAL(valueChanged(int)), component(), SLOT(setRepeats(int)));

  connect(ui->start, SIGNAL(clicked()), SLOT(onStartStop()));
  connect(ui->open, SIGNAL(clicked()), SLOT(onOpenClose()));

  connect(component(), SIGNAL(exposureChanged(int)), ui->exposure, SLOT(setValue(int)));
  connect(component(), SIGNAL(cycleChanged(int)), ui->cycle, SLOT(setValue(int)));
  connect(component(), SIGNAL(minCycleChanged(int)), ui->cycle, SLOT(setMin(int)));
  connect(component(), SIGNAL(repeatsChanged(int)), ui->repititions, SLOT(setValue(int)));
  connect(component(), SIGNAL(repeatsChanged(int)), ui->progressBar, SLOT(setMaximum(int)));
  connect(component(), SIGNAL(progressChanged(int)), SLOT(updateProgress(int)));
  connect(component(), SIGNAL(canStartChanged(bool)), SLOT(updateCanStart(bool)));
  connect(component(), SIGNAL(valuesOKchanged(bool)), SLOT(updateValuesOK(bool)));
  connect(component(), SIGNAL(stateChanged(MrtShutter::State)), SLOT(updateState(MrtShutter::State)));

  updateConnection(component()->isConnected());

}

void MrtShutterGui::updateConnection(bool con) {
  setEnabled(con);
  if (con) {
    updateState(component()->state());
    updateValuesOK(component()->valuesOK());
    updateProgress(component()->progress());
    updateCanStart(component()->canStart());
    ui->exposure->setValue(component()->exposure());
    ui->cycle->setValue(component()->cycle());
    ui->repititions->setValue(component()->repeats());
  } else {
    ui->start->setText("Disconnected");
    ui->open->setText("Disconnected");
  }
}

void MrtShutterGui::updateProgress(int prog) {
  ui->progressBar->setVisible(prog);
  ui->progressBar->setValue(prog-1);
  ui->start->setText( prog ? "Stop" : "Expose" );
  updateCanStart(component()->canStart());
}

void MrtShutterGui::updateCanStart(bool can) {
  ui->start->setEnabled( can || component()->progress() );
}

void MrtShutterGui::updateState(MrtShutter::State state) {
  switch (state) {
  case MrtShutter::OPENED :
    ui->open->setText("Close");
    break;
  case MrtShutter::CLOSED :
    ui->open->setText("Open");
    break;
  case MrtShutter::BETWEEN :
    ui->open->setText("In progress");
    break;
  }
}

void MrtShutterGui::updateValuesOK(bool ok) {
  static const QString nonOKstyle = "background-color: rgba(255, 0, 0, 128);";
  ui->cycle->setStyleSheet(ok ? "" : nonOKstyle);
  ui->exposure->setStyleSheet(ok ? "" : nonOKstyle);
}


void MrtShutterGui::onOpenClose() {
  if (component()->state() == MrtShutter::CLOSED)
    component()->open();
  else if (component()->state() == MrtShutter::OPENED)
    component()->close();
  else component()->close();
}

void MrtShutterGui::onStartStop() {
  if (component()->progress())
    component()->stop();
  else
    component()->start();
}
