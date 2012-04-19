#include "monoGui.h"
#include "error.h"
#include "ui_monoGui.h"
#include "tuner.h"




EnergySetRevert::EnergySetRevert(QWidget * master) :
  QWidget(static_cast<QWidget*>(master->parent())),
  setBut(new QPushButton("Set",this)),
  revertBut(new QPushButton("Revert",this))
{
  hide();
  setAutoFillBackground(true);
  QHBoxLayout * lay = new QHBoxLayout;
  lay->addWidget(setBut);
  lay->addWidget(revertBut);
  setLayout(lay);
  connect(setBut, SIGNAL(clicked()), SIGNAL(set()));
  connect(revertBut, SIGNAL(clicked()), SIGNAL(revert()));
  master->installEventFilter(this);
}


bool EnergySetRevert::eventFilter(QObject *obj, QEvent *event) {
  QWidget * objW = static_cast<QWidget*>(obj);
  if (objW) {
    if ( event->type() == QEvent::Resize ) {
      setMinimumWidth(objW->width());
      setMaximumWidth(objW->width());
    } else if ( event->type() == QEvent::Move ||
                event->type() == QEvent::Show ) {
      QRect geom = geometry();
      geom.moveTopLeft(objW->geometry().topLeft()+QPoint(0, objW->height()));
      setGeometry(geom);
    }
  }
  return QObject::eventFilter(obj, event);
}



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

  int apbrow, neverused;
  ui->mainLayout->getItemPosition ( ui->mainLayout->indexOf(ui->advanced_pb),
                                    &apbrow, &neverused, &neverused, &neverused);
  for (int row = apbrow+1 ; row < ui->mainLayout->rowCount() - 1 ; row++ )
    for ( int column = 0; column < ui->mainLayout->columnCount() ; column++ )
      if ( ui->mainLayout->itemAtPosition(row,column) &&
           ui->mainLayout->itemAtPosition(row,column)->widget() )
        advancedWidgets << ui->mainLayout->itemAtPosition(row,column)->widget();

  energySetter = new EnergySetRevert(ui->energy);
  ui->bigDBrag->hide();
  ui->bigDX->hide();

  ui->energy->setRange(Mono::energyRange.first, Mono::energyRange.second);

  ui->motors->lock(true);
  ui->motors->addMotor(component()->motors.values(),true,true);

  connect(component(), SIGNAL(connectionChanged(bool)), SLOT(updateConnection(bool)));
  connect(component(), SIGNAL(motionChanged(bool)), SLOT(updateMotion(bool)));
  connect(component(), SIGNAL(energyChanged(double)), SLOT(updateEnergy()));
  connect(component(), SIGNAL(dBraggChanged(double)), SLOT(updateDBragg()));
  connect(component(), SIGNAL(dXChanged(double)), SLOT(updateDX()));
  connect(component(), SIGNAL(zSeparationChanged(double)), ui->zSeparation, SLOT(setValue(double)));
  connect(component(), SIGNAL(dZChanged(double)), SLOT(updateDZ()));
  connect(component(), SIGNAL(tilt1Changed(double)), SLOT(updateTilt1()));
  connect(component(), SIGNAL(tilt2Changed(double)), SLOT(updateTilt2()));
  connect(component(), SIGNAL(bend1frontChanged(double)), SLOT(updateBend1f()));
  connect(component(), SIGNAL(bend2frontChanged(double)), SLOT(updateBend2f()));
  connect(component(), SIGNAL(bend1backChanged(double)), SLOT(updateBend1b()));
  connect(component(), SIGNAL(bend2backChanged(double)), SLOT(updateBend2b()));
  connect(component(), SIGNAL(inBeamChanged(Mono::InOutPosition)), SLOT(updateInOut(Mono::InOutPosition)));

  connect(component()->motors[Mono::Bragg2], SIGNAL(changedUserPosition(double)), SLOT(updateMotorBragg2()));
  connect(component()->motors[Mono::Bragg1], SIGNAL(changedUserPosition(double)), SLOT(updateMotorBragg1()));
  connect(component()->motors[Mono::Xdist], SIGNAL(changedUserPosition(double)), SLOT(updateMotorX()));

  foreach(QCaMotor * mot, component()->motors.values()) {
    connect(mot, SIGNAL(changedHiLimitStatus(bool)), SLOT(updateLSs()));
    connect(mot, SIGNAL(changedLoLimitStatus(bool)), SLOT(updateLSs()));
  }

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->si111, SIGNAL(toggled(bool)),  SLOT(onEnergyTune()));
  connect(ui->si311, SIGNAL(toggled(bool)),  SLOT(onEnergyTune()));
  connect(ui->energy, SIGNAL(valueChanged(double)),  SLOT(onEnergyTune()));
  connect(energySetter, SIGNAL(set()),  SLOT(onEnergySet()));
  connect(energySetter, SIGNAL(revert()),  SLOT(updateEnergy()));
  connect(ui->tuneBragg, SIGNAL(valueEdited(double)), component(), SLOT(setDBragg(double)));
  connect(ui->tuneX, SIGNAL(valueEdited(double)), component(), SLOT(setDX(double)));
  connect(ui->bend1front, SIGNAL(valueEdited(double)), component(), SLOT(setBend1front(double)));
  connect(ui->bend2front, SIGNAL(valueEdited(double)), component(), SLOT(setBend2front(double)));
  connect(ui->bend1back, SIGNAL(valueEdited(double)), component(), SLOT(setBend1back(double)));
  connect(ui->bend2back, SIGNAL(valueEdited(double)), component(), SLOT(setBend2back(double)));
  connect(ui->tilt1, SIGNAL(valueEdited(double)), component(), SLOT(setTilt1(double)));
  connect(ui->tilt2, SIGNAL(valueEdited(double)), component(), SLOT(setTilt2(double)));
  connect(ui->zSeparation, SIGNAL(valueEdited(double)), SLOT(onZseparationSet()));
  connect(ui->moveIn, SIGNAL(clicked()), component(), SLOT(moveIn()));
  connect(ui->moveOut, SIGNAL(clicked()), component(), SLOT(moveOut()));
  connect(ui->stop, SIGNAL(clicked()), component(), SLOT(stop()));

  updateConnection(component()->isConnected());
  updateMotion(false);
  updateEnergy();
  updateMotorBragg1();
  updateMotorBragg2();
  updateMotorX();
  onAdvancedControl();

}

MonoGui::~MonoGui() {
    delete ui;
}


void MonoGui::updateMotion(bool moving) {
  ui->stop->setEnabled( component()->isConnected() && moving);
  if(!moving) {
    updateDBragg();
    updateDX();
    updateDZ();
    updateTilt1();
    updateTilt2();
    updateBend1f();
    updateBend1b();
    updateBend2f();
    updateBend2b();
  }
}



void MonoGui::updateConnection(bool con) {
   ui->mainWidget->setEnabled(component()->isConnected());
  ui->stop->setText( con ? "Stop all" : "No link");
  ui->stop->setStyleSheet( con ? "" : "color: rgb(255, 0, 0); background-color: rgb(0, 0, 0);");
  updateMotion(component()->isMoving());
  if (con) {
    ui->tuneBragg->setIncrement(component()->motors[Mono::Bragg1]->getStep());
    ui->tuneX->setIncrement(component()->motors[Mono::Xdist]->getStep());
    ui->tilt1->setIncrement(component()->motors[Mono::Tilt1]->getStep());
    ui->tilt2->setIncrement(component()->motors[Mono::Tilt2]->getStep());
    ui->bend1front->setIncrement(component()->motors[Mono::Bend1f]->getStep());
    ui->bend1back->setIncrement(component()->motors[Mono::Bend1b]->getStep());
    ui->bend2front->setIncrement(component()->motors[Mono::Bend2f]->getStep());
    ui->bend2back->setIncrement(component()->motors[Mono::Bend2b]->getStep());
    ui->tuneZ->setIncrement(component()->motors[Mono::Z2]->getStep());
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
  bool setrevert =
      ! component()->motors[Mono::Bragg1]->isMoving() &&
      ! component()->motors[Mono::Bragg2]->isMoving() &&
      ! component()->motors[Mono::Xdist]->isMoving() &&
      ( enrg != component()->energy() ||
      ( ui->si111->isChecked() && component()->diffraction() != Mono::Si111 ) ||
      ( ui->si311->isChecked() && component()->diffraction() != Mono::Si311 ) );
  energySetter->setVisible(setrevert);
}


void MonoGui::onEnergySet() {
  component()->setEnergy(ui->energy->value(),
                         ui->si111->isChecked() ? Mono::Si111 : Mono::Si311,
                         ui->lockBragg->isChecked(), ui->lockX->isChecked());
  energySetter->hide();
}


void MonoGui::onZseparationSet() {
  component()->setZseparation(ui->zSeparation->value(), ui->lockDZ->isChecked());
}


void MonoGui::updateEnergy() {
  if ( qAbs(ui->energy->value() - component()->energy()) >= 1.0e-03 )
    ui->energy->setValue(component()->energy());
  if (component()->diffraction() == Mono::Si111)
    ui->si111->setChecked(true);
  else
    ui->si311->setChecked(true);
  energySetter->hide();
}


void MonoGui::updateInOut(Mono::InOutPosition iopos) {
  ui->currentInOut->setStyleSheet("");
  switch (iopos) {
  case Mono::INBEAM : ui->currentInOut->setText("In"); break;
  case Mono::OUTBEAM : ui->currentInOut->setText("Out"); break;
  case Mono::BETWEEN :
    ui->currentInOut->setText("between: " +
                              QString::number(component()->motors[Mono::Z1]->getUserPosition()));
    ui->currentInOut->setStyleSheet("color: rgba(255, 0, 0);");
    break;
  case Mono::MOVING :
    ui->currentInOut->setText("moving: " +
                              QString::number(component()->motors[Mono::Z1]->getUserPosition()));
    break;
  }
}


void MonoGui::updateDBragg() {
  ui->readBragg->setText(QString::number(component()->dBragg()));
  if ( ! component()->motors[Mono::Bragg1]->isMoving() &&
       ! component()->motors[Mono::Bragg2]->isMoving() )
    ui->tuneBragg->setValue(component()->dBragg());
  ui->bigDBrag->setVisible( component()->dBragg() < ui->tuneBragg->minimum() ||
                            component()->dBragg() > ui->tuneBragg->maximum() );
}


void MonoGui::updateDX() {
  ui->readDX->setText(QString::number(component()->dX()));
  if ( ! component()->motors[Mono::Bragg2]->isMoving() &&
       ! component()->motors[Mono::Xdist]->isMoving() )
    ui->tuneX->setValue(component()->dX());
  ui->bigDX->setVisible( component()->dX() < ui->tuneX->minimum() ||
                         component()->dX() > ui->tuneX->maximum() );
}


void MonoGui::updateDZ() {
  if ( ! component()->motors[Mono::Z2]->isMoving() )
    ui->tuneZ->setValue(component()->zTweak());
}

void MonoGui::updateTilt1() {
  ui->readTilt1->setText(QString::number(component()->tilt1()));
  if ( ! component()->motors[Mono::Tilt1]->isMoving() )
    ui->tilt1->setValue(component()->tilt1());
}


void MonoGui::updateTilt2() {
  ui->readTilt2->setText(QString::number(component()->tilt2()));
  if ( ! component()->motors[Mono::Tilt2]->isMoving() )
    ui->tilt2->setValue(component()->tilt2());
}


void MonoGui::updateBend1f() {
  ui->readB1F->setText(QString::number(component()->bend1front()));
  if ( ! component()->motors[Mono::Bend1f]->isMoving() )
    ui->bend1front->setValue(component()->bend1front());
}


void MonoGui::updateBend1b() {
  ui->readB1B->setText(QString::number(component()->bend1back()));
  if ( ! component()->motors[Mono::Bend1b]->isMoving() )
    ui->bend1back->setValue(component()->bend1back());
}

void MonoGui::updateBend2f() {
  ui->readB2F->setText(QString::number(component()->bend2front()));
  if ( ! component()->motors[Mono::Bend2f]->isMoving() )
    ui->bend2front->setValue(component()->bend2front());
}


void MonoGui::updateBend2b() {
  ui->readB2B->setText(QString::number(component()->bend2back()));
  if ( ! component()->motors[Mono::Bend2b]->isMoving() )
    ui->bend2back->setValue(component()->bend2back());
}



void MonoGui::onAdvancedControl() {
  if (ui->motors->isVisibleTo(this)) {
    foreach (QWidget * wdg, advancedWidgets)
      wdg->setVisible(false);
    ui->advanced_pb->setText("Show advanced control");
    ui->advanced_pb->setStyleSheet("");
  } else if ( PsswDial::ask(this) ) {
    foreach (QWidget * wdg, advancedWidgets)
      wdg->setVisible(true);
    ui->advanced_pb->setText("Please don't forget to close");
    ui->advanced_pb->setStyleSheet("background-color: rgba(255, 0, 0,64);");
  }
}


void MonoGui::updateMotorBragg1() {
  QString msg;
  msg += QString().sprintf("Crystal 1: %f", component()->motors[Mono::Bragg1]->getUserPosition());
  if ( component()->motors[Mono::Bragg1]->isMoving() )
    msg += QString().sprintf( " -> %f", component()->motors[Mono::Bragg1]->getUserGoal());
  ui->readBragg1->setText(msg);
}

void MonoGui::updateMotorBragg2() {
  QString msg;
  msg += QString().sprintf("Crystal 2: %f", component()->motors[Mono::Bragg2]->getUserPosition());
  if ( component()->motors[Mono::Bragg2]->isMoving() )
    msg += QString().sprintf( " -> %f", component()->motors[Mono::Bragg2]->getUserGoal());
  ui->readBragg2->setText(msg);
}

void MonoGui::updateMotorX() {
  QString msg;
  msg += QString().sprintf("X: %f", component()->motors[Mono::Xdist]->getUserPosition());
  if ( component()->motors[Mono::Xdist]->isMoving() )
    msg += QString().sprintf( " -> %f", component()->motors[Mono::Xdist]->getUserGoal());
  ui->readX->setText(msg);
}


void MonoGui::updateLSs() {
  QCaMotor * mot = static_cast<QCaMotor*>(sender());
  if (!mot)
    return;
  QLabel * lab=0;
  if (mot == component()->motors[Mono::Xdist]) lab = ui->readX;
  else if (mot == component()->motors[Mono::Bragg1]) lab = ui->readBragg1;
  else if (mot == component()->motors[Mono::Bragg2]) lab = ui->readBragg2;
  else if (mot == component()->motors[Mono::Tilt1]) lab = ui->readTilt1;
  else if (mot == component()->motors[Mono::Tilt2]) lab = ui->readTilt2;
  else if (mot == component()->motors[Mono::Z1]) lab = ui->currentInOut;
  else if (mot == component()->motors[Mono::Z2]) lab = ui->readDZ;
  else if (mot == component()->motors[Mono::Bend1f]) lab = ui->readB1F;
  else if (mot == component()->motors[Mono::Bend1b]) lab = ui->readB1B;
  else if (mot == component()->motors[Mono::Bend2f]) lab = ui->readB2F;
  else if (mot == component()->motors[Mono::Bend2b]) lab = ui->readB2B;
  else return;
  lab->setStyleSheet(
        (mot->getHiLimitStatus() || mot->getLoLimitStatus()) ?
          "background-color: rgb(128, 0, 0); color: rgb(255, 255, 255);" :
          "");
  }
