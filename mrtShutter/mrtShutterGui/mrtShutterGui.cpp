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

  connect(ui->cycle, SIGNAL(valueEdited(double)), component(), SLOT(setCycle(double)));
  connect(ui->exposure, SIGNAL(valueEdited(double)), component(), SLOT(setExposure(double)));
  connect(ui->repititions, SIGNAL(valueChanged(int)), component(), SLOT(setRepeats(int)));
  connect(ui->powerReset, SIGNAL(clicked()), component(), SLOT(resetPower()));

  connect(ui->start, SIGNAL(clicked()), SLOT(onStartStop()));
  connect(ui->open, SIGNAL(clicked()), SLOT(onOpenClose()));
  connect(ui->exposureMode, SIGNAL(activated(int)), SLOT(onExposureMode()));
  connect(ui->trig, SIGNAL(clicked()), SLOT(onTrig()));

  connect(component(), SIGNAL(exposureChanged(double)), ui->exposure, SLOT(setValue(double)));
  connect(component(), SIGNAL(cycleChanged(double)), ui->cycle, SLOT(setValue(double)));
  connect(component(), SIGNAL(minRelaxChanged(double)), ui->cycle, SLOT(setMin(double)));
  connect(component(), SIGNAL(repeatsChanged(int)), ui->repititions, SLOT(setValue(int)));
  connect(component(), SIGNAL(repeatsChanged(int)), ui->progressBar, SLOT(setMaximum(int)));
  connect(component(), SIGNAL(progressChanged(int)), SLOT(updateProgress(int)));
  connect(component(), SIGNAL(canStartChanged(bool)), SLOT(updateCanStart(bool)));
  connect(component(), SIGNAL(valuesOKchanged(bool)), SLOT(updateValuesOK(bool)));
  connect(component(), SIGNAL(stateChanged(MrtShutter::State)),
          SLOT(updateState(MrtShutter::State)));
  connect(component(), SIGNAL(exposureModeChanged(MrtShutter::ExposureMode)),
          SLOT(updateExposureMode(MrtShutter::ExposureMode)));
  connect(component(), SIGNAL(powerStatusChanged(QString)), ui->powerState, SLOT(setText(QString)) );

  updateConnection(component()->isConnected());

}

void MrtShutterGui::updateConnection(bool con) {
  setEnabled(con);
  if (con) {
    updateState(component()->state());
    updateValuesOK(component()->valuesOK());
    wasProg = ! (bool) component()->progress();
    updateProgress(component()->progress());
    updateCanStart(component()->canStart());
    updateExposureMode(component()->exposureMode());
    ui->exposure->setValue(component()->exposure());
    ui->cycle->setValue(component()->cycle());
    ui->repititions->setValue(component()->repeats());
    ui->trig->setText("Trig");
  } else {
    ui->start->setText("No link");
    ui->open->setText("No link");
    ui->trig->setText("No link");
  }
}

void MrtShutterGui::updateProgress(int prog) {

  ui->progressBar->setValue(prog-1);
  ui->trig->setEnabled(true);

  ui->start->setText( prog ? "Stop" : "Expose" );

  if ( wasProg == (bool) prog )
    return;
  wasProg = (bool) prog;


  ui->progressBar->setVisible( prog &&
                               component()->exposureMode() == MrtShutter::TIME );
  ui->open->setVisible( ! prog ||
                        component()->exposureMode() != MrtShutter::SOFT );
  ui->open->setEnabled(!prog);
  ui->trig->setVisible(prog &&
                       component()->exposureMode() == MrtShutter::SOFT);

  ui->exposure->setEnabled(!prog);
  ui->timerParameters->setEnabled(!prog);
  ui->exposureMode->setEnabled(!prog);

  updateCanStart(component()->canStart());

}

void MrtShutterGui::updateCanStart(bool can) {
  ui->start->setEnabled( can || component()->progress() );
}

void MrtShutterGui::updateState(MrtShutter::State state) {
  switch (state) {
  case MrtShutter::OPENED :
  case MrtShutter::BETWEEN :
    ui->open->setText("Close");
    break;
  case MrtShutter::CLOSED :
    ui->open->setText("Open");
    break;
  case MrtShutter::EXPOSING :
    ui->open->setText("Exposing");
    break;

  }
}

void MrtShutterGui::updateExposureMode(MrtShutter::ExposureMode mode) {
  ui->timerParameters->setVisible(mode == MrtShutter::TIME);
  ui->exposureMode->setCurrentIndex( (int) mode );
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
  else {
    const double maxrepeats = 10000;
    if (component()->exposureMode() != MrtShutter::TIME &&
        component()->repeats() != maxrepeats ) {
      component()->setRepeats(maxrepeats);
      qtWait(component(), SIGNAL(repeatsChanged(int)), 500);
    }
    component()->start(true);
  }
}

void MrtShutterGui::onTrig() {
  if ( ! component()->progress() ||
       component()->exposureMode() != MrtShutter::SOFT )
    return;
  ui->trig->setEnabled(false); // the updateProgress will reenable it
  component()->trig(false);
}

void MrtShutterGui::onExposureMode() {
  component()->setExposureMode(
        (MrtShutter::ExposureMode) ui->exposureMode->currentIndex() );
}
