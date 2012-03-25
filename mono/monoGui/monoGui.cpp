#include "monoGui.h"
#include "error.h"
#include "ui_monoGui.h"
#include "tuner.h"


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
  ui->motors->addMotor(component()->motors.values(),true,true);

  connect(component(), SIGNAL(connectionChanged(bool)), SLOT(updateConnection(bool)));
  connect(component(), SIGNAL(motionChanged(bool)), SLOT(updateMotion(bool)));
  connect(component(), SIGNAL(energyChanged(double)), SLOT(updateEnergy()));
  connect(component(), SIGNAL(dBraggChanged(double)), ui->tuneBragg, SLOT(setValue(double)));
  connect(component(), SIGNAL(dXChanged(double)), ui->tuneX, SLOT(setValue(double)));
  connect(component(), SIGNAL(zSeparationChanged(double)), ui->z2tuner, SLOT(setValue(double)));
  connect(component(), SIGNAL(tilt1Changed(double)), ui->tilt1, SLOT(setValue(double)));
  connect(component(), SIGNAL(tilt2Changed(double)), ui->tilt2, SLOT(setValue(double)));
  connect(component(), SIGNAL(bend1frontChanged(double)), ui->bend1front, SLOT(setValue(double)));
  connect(component(), SIGNAL(bend2frontChanged(double)), ui->bend2front, SLOT(setValue(double)));
  connect(component(), SIGNAL(bend1backChanged(double)), ui->bend1back, SLOT(setValue(double)));
  connect(component(), SIGNAL(bend2backChanged(double)), ui->bend2back, SLOT(setValue(double)));
  connect(component(), SIGNAL(inBeamChanged(Mono::InOutPosition)), SLOT(updateInOut(Mono::InOutPosition)));

  connect(component()->motors[Mono::Bragg2], SIGNAL(changedMoving(bool)), SLOT(updateEnergyChanging()));

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->si111, SIGNAL(toggled(bool)),  SLOT(onEnergyTune()));
  connect(ui->si311, SIGNAL(toggled(bool)),  SLOT(onEnergyTune()));
  connect(ui->energy, SIGNAL(valueChanged(double)),  SLOT(onEnergyTune()));
  connect(ui->enSet, SIGNAL(clicked()),  SLOT(onEnergySet()));
  connect(ui->enRevert, SIGNAL(clicked()),  SLOT(updateEnergy()));
  connect(ui->tuneBragg, SIGNAL(valueEdited(double)), component(), SLOT(setDBragg(double)));
  connect(ui->tuneX, SIGNAL(valueEdited(double)), component(), SLOT(setDX(double)));
  connect(ui->bend1front, SIGNAL(valueEdited(double)), component(), SLOT(setBend1front(double)));
  connect(ui->bend2front, SIGNAL(valueEdited(double)), component(), SLOT(setBend2front(double)));
  connect(ui->bend1back, SIGNAL(valueEdited(double)), component(), SLOT(setBend1back(double)));
  connect(ui->bend2back, SIGNAL(valueEdited(double)), component(), SLOT(setBend2back(double)));
  connect(ui->tilt1, SIGNAL(valueEdited(double)), component(), SLOT(setTilt1(double)));
  connect(ui->tilt2, SIGNAL(valueEdited(double)), component(), SLOT(setTilt2(double)));
  connect(ui->z2tuner, SIGNAL(valueEdited(double)), component(), SLOT(setZseparation(double)));
  connect(ui->moveIn, SIGNAL(clicked()), component(), SLOT(moveIn()));
  connect(ui->moveOut, SIGNAL(clicked()), component(), SLOT(moveOut()));
  connect(ui->stop, SIGNAL(clicked()), component(), SLOT(stop()));

  updateConnection(component()->isConnected());
  updateEnergy();
  updateEnergyChanging();

}

MonoGui::~MonoGui() {
    delete ui;
}


void MonoGui::updateMotion(bool moving) {
  /*
  ui->stop->setText(moving?"Stop":"Reset");
  */
  ui->mainLayout->setEnabled(!moving);
  ui->advanced_pb->setVisible(!moving);
  ui->stop->setVisible(moving);
}

void MonoGui::updateEnergyChanging() {
  if (component()->motors[Mono::Bragg2]->isMoving()) {
    ui->currentThetas->setVisible(true);
    ui->currentThetas->setText(
          "Crystal 1: " +
          QString::number(component()->motors[Mono::Bragg1]->getUserPosition()) + "->" +
          QString::number(component()->motors[Mono::Bragg1]->getUserGoal()) + ", "
          "Crystal 2: " +
          QString::number(component()->motors[Mono::Bragg2]->getUserPosition()) + "->" +
          QString::number(component()->motors[Mono::Bragg2]->getUserGoal()) + ", "
          "X: " +
          QString::number(component()->motors[Mono::Xdist]->getUserPosition()) + "->" +
          QString::number(component()->motors[Mono::Xdist]->getUserGoal()) );
  } else
    ui->currentThetas->setVisible(false);
}



void MonoGui::updateConnection(bool con) {
  /*
  ui->mainWdg->setEnabled(con);
  ui->z2tuner->setEnabled(con);
  ui->inout->setEnabled(con);
  ui->stop->setEnabled(con);
  */
  if (con) {
    updateMotion(component()->isMoving());
  } else {
    ui->currentInOut->setText("disconnected");
    ui->stop->setText("Disconnected");
  }
}

void MonoGui::onEnergyTune() {
  const double enrg = ui->energy->value();
  ui->si311->setDisabled(enrg < Mono::minEnergy311);
  ui->si111->setDisabled(enrg > Mono::maxEnergy111);
  if ( enrg < Mono::minEnergy311 && ui->si311->isChecked() )
    ui->si111->setChecked(true);
  if ( enrg > Mono::maxEnergy111 && ui->si111->isChecked() )
    ui->si311->setChecked(true);
  bool setrevert = enrg != component()->energy() ||
      ( ui->si111->isChecked() && component()->diffraction() != Mono::Si111 ) ||
      ( ui->si311->isChecked() && component()->diffraction() != Mono::Si311 );
  ui->enRevert->setVisible(setrevert);
  ui->enSet->setVisible(setrevert);
}


void MonoGui::onEnergySet() {
  component()->setEnergy(ui->energy->value(),
                         ui->si111->isChecked() ? Mono::Si111 : Mono::Si311,
                         ui->lockBragg->isChecked(), ui->lockX->isChecked());
  ui->enRevert->hide();
  ui->enSet->hide();
}


void MonoGui::updateEnergy() {
  ui->energy->setValue(component()->energy());
  if (component()->diffraction() == Mono::Si111)
    ui->si111->setChecked(true);
  else
    ui->si311->setChecked(true);
  ui->enRevert->hide();
  ui->enSet->hide();
}


void MonoGui::updateInOut(Mono::InOutPosition iopos) {
  switch (iopos) {
  case Mono::INBEAM : ui->currentInOut->setText("In"); break;
  case Mono::OUTBEAM : ui->currentInOut->setText("Out"); break;
  case Mono::BETWEEN : ui->currentInOut->setText("between"); break;
  case Mono::MOVING : ui->currentInOut->setText("moving"); break;
  }
  ui->currentInOut->setStyleSheet( iopos == Mono::BETWEEN ?
                                     "background-color: rgba(255, 0, 0,64);" :
                                     "");
}

/*
void MonoGui::onStop() {
  if (component()->isMoving())
    component()->stop();
  else {
    updateInOut(component()->inBeam());
    updateEnergy();
    ui->tilt1->setValue(component()->tilt1());
    ui->tilt2->setValue(component()->tilt2());
    ui->bend1front->setValue(component()->bend1front());
    ui->bend2front->setValue(component()->bend2front());
    ui->bend1back->setValue(component()->bend1back());
    ui->bend2back->setValue(component()->bend2back());
    ui->z2tuner->setValue(component()->zSeparation());
    ui->tuneBragg->setValue(component()->dBragg());
    ui->tuneX->setValue(component()->dX());
  }
}
*/


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


