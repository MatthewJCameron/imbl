#include "monoGui.h"
#include "error.h"
#include "ui_monoGui.h"


MonoGui::MonoGui(QWidget *parent) :
  ComponentGui(new Mono(parent), true, parent),
  ui(new Ui::MonoGui)
{
  init();
}

MonoGui::MonoGui(Mono * mono, QWidget *parent) :
  ComponentGui(mono, true, parent),
  ui(new Ui::MonoGui)
{
  init();
}


void MonoGui::init() {

  ui->setupUi(this);
  ui->advanced->hide();

  ui->energy->setRange(Mono::energyRange.first, Mono::energyRange.second);

  ui->motors->lock(true);
  ui->motors->addMotor(component()->listMotors(),true,true);

  connect(component(), SIGNAL(connectionChanged(bool)), SLOT(updateConnection(bool)));
  connect(component(), SIGNAL(motionChanged(bool)), SLOT(updateMotion(bool)));
  connect(component(), SIGNAL(energyChanged(double)), SLOT(updateEnergy()));
  connect(component(), SIGNAL(dBraggChanged(double)), ui->tuneBragg, SLOT(setValue(double)));
  connect(component(), SIGNAL(dXChanged(double)), ui->tuneX, SLOT(setValue(double)));
  connect(component(), SIGNAL(dZChanged(double)), ui->z2tuner, SLOT(setValue(double)));
  connect(component(), SIGNAL(tilt1Changed(double)), ui->tilt1, SLOT(setValue(double)));
  connect(component(), SIGNAL(tilt2Changed(double)), ui->tilt2, SLOT(setValue(double)));
  connect(component(), SIGNAL(bend1Changed(double)), ui->bend1, SLOT(setValue(double)));
  connect(component(), SIGNAL(bend2Changed(double)), ui->bend2, SLOT(setValue(double)));
  connect(component(), SIGNAL(inBeamChanged(Mono::InOutPosition)), SLOT(updateInOut(Mono::InOutPosition)));

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->si111, SIGNAL(toggled(bool)),  SLOT(onEnergyTune()));
  connect(ui->si311, SIGNAL(toggled(bool)),  SLOT(onEnergyTune()));
  connect(ui->energy, SIGNAL(valueChanged(double)),  SLOT(onEnergyTune()));
  connect(ui->energy, SIGNAL(valueEdited(double)),  SLOT(onEnergySet()));
  connect(ui->tuneBragg, SIGNAL(valueChanged(double)), component(), SLOT(setDBragg(double)));
  connect(ui->tuneX, SIGNAL(valueChanged(double)), component(), SLOT(setDX(double)));
  connect(ui->bend1, SIGNAL(valueChanged(double)), component(), SLOT(setBend1(double)));
  connect(ui->bend2, SIGNAL(valueChanged(double)), component(), SLOT(setBend2(double)));
  connect(ui->tilt1, SIGNAL(valueChanged(double)), component(), SLOT(setTilt1(double)));
  connect(ui->tilt2, SIGNAL(valueChanged(double)), component(), SLOT(setTilt2(double)));
  connect(ui->z2tuner, SIGNAL(valueChanged(double)), component(), SLOT(setDZ(double)));
  connect(ui->moveIn, SIGNAL(clicked()), component(), SLOT(moveIn()));
  connect(ui->moveOut, SIGNAL(clicked()), component(), SLOT(moveOut()));
  connect(ui->stop, SIGNAL(clicked()), SLOT(onStopReset()));

  updateConnection(component()->isConnected());
  updateEnergy();

}

MonoGui::~MonoGui() {
    delete ui;
}


void MonoGui::updateMotion(bool moving) {
  ui->stop->setText(moving?"Stop":"Reset");
  ui->mainWdg->setEnabled(!moving);
}


void MonoGui::updateConnection(bool con) {
  ui->mainWdg->setEnabled(con);
  ui->z2tuner->setEnabled(con);
  ui->inout->setEnabled(con);
  ui->stop->setEnabled(con);
  ui->stop->setText("Disconnected");
  if (con)
    updateMotion(component()->isMoving());
  else
    ui->currentInOut->setText("disconnected");
}


void MonoGui::onEnergyTune() {
  const double enrg = ui->energy->value();
  ui->wave->setValue( 12.398419 / enrg ); // 12.398419 = h*c (keV)
  ui->si311->setDisabled(enrg < Mono::minEnergy311);
  ui->si111->setDisabled(enrg > Mono::maxEnergy111);
  if ( enrg < Mono::minEnergy311 && ui->si311->isChecked() )
    ui->si111->setChecked(true);
  if ( enrg > Mono::maxEnergy111 && ui->si111->isChecked() )
    ui->si311->setChecked(true);
  ui->angle->setValue(
        energy2bragg(enrg, ui->si111->isChecked() ? Mono::Si111 : Mono::Si311) );
}


void MonoGui::onEnergySet() {
  component()->setEnergy(ui->energy->value(),
                         ui->si111->isChecked() ? Mono::Si111 : Mono::Si311,
                         ui->lockBragg->isChecked(), ui->lockX->isChecked());
}

void MonoGui::updateEnergy() {
  ui->energy->setValue(component()->energy());
  if (component()->diffraction() == Mono::Si111)
    ui->si111->setChecked(true);
  else
    ui->si311->setChecked(true);
}


void MonoGui::updateInOut(Mono::InOutPosition iopos) {
  switch (iopos) {
  case Mono::INBEAM : ui->currentInOut->setText("In"); break;
  case Mono::OUTBEAM : ui->currentInOut->setText("Out"); break;
  case Mono::BETWEEN : ui->currentInOut->setText("between"); break;
  case Mono::MOVING : ui->currentInOut->setText("moving"); break;
  }
  ui->currentInOut->setStyleSheet( iopos == Mono::MOVING ?
                                     "background-color: rgba(255, 0, 0,64);":
                                     "");
}


void MonoGui::onStopReset() {
  if (component()->isMoving())
    component()->stop();
  else {
    updateInOut(component()->inBeam());
    updateEnergy();
    ui->tilt1->setValue(component()->tilt1());
    ui->tilt2->setValue(component()->tilt2());
    ui->bend1->setValue(component()->bend1());
    ui->bend2->setValue(component()->bend2());
    ui->z2tuner->setValue(component()->dZ());
    ui->tuneBragg->setValue(component()->dBragg());
    ui->tuneX->setValue(component()->dX());
  }
}


void MonoGui::onAdvancedControl() {
  if (ui->advanced->isVisible()) {
    ui->advanced->hide();
    ui->advanced_pb->setText("Show advanced control");
    ui->advanced_pb->setStyleSheet("");
  } else if ( PsswDial::ask(this) ) {
    ui->advanced->show();
    ui->advanced_pb->setText("Please don't forget to close");
    ui->advanced_pb->setStyleSheet("background-color: rgba(255, 0, 0,64);");
  }
}


