#include "monoGui.h"
#include "error.h"
#include "ui_monoGui.h"
#include "ui_wtf.h"
#include "tuner.h"



EnergySetRevert::EnergySetRevert(QWidget * master) :
  QWidget(static_cast<QWidget*>(master->parent())),
  setBut(new QPushButton("Set",this)),
  revertBut(new QPushButton("Revert",this))
{
  hide();
  setAutoFillBackground(true);
  QHBoxLayout * lay = new QHBoxLayout;
  lay->setContentsMargins(0, 0, 0, 0);
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
      geom.moveTopLeft(objW->geometry().topLeft()-QPoint(0, height()));
      setGeometry(geom);
    } else if ( event->type() == QEvent::KeyPress ) {
      int key = static_cast<QKeyEvent*>(event)->key();
      if ( key == Qt::Key_Enter || key == Qt::Key_Return ) {
        setBut->setFocus();
        setBut->click();
      } else if ( key == Qt::Key_Escape ) {
        revertBut->setFocus();
        revertBut->click();
      }
    }
  }
  return QObject::eventFilter(obj, event);
}










MonoGui::MonoGui(QWidget *parent) :
  ComponentGui(new Mono(parent), true, parent),
  ui(new Ui::MonoGui),
  wtfUi(new Ui::WTF),
  calibrateDialog(new QDialog(this)),
  wtfDialog(new QDialog(this))
{
  init();
}

MonoGui::MonoGui(Mono * mono, QWidget *parent) :
  ComponentGui(mono, true, parent),
  ui(new Ui::MonoGui),
  wtfUi(new Ui::WTF),
  calibrateDialog(new QDialog(this)),
  wtfDialog(new QDialog(this))
{
  init();
}


void MonoGui::init() {

  dBraggHealth=true;
  dXhealth=true;
  dZhealth=true;

  ui->setupUi(this);
  ui->motors_2->installEventFilter(this);
  ui->motors_1->installEventFilter(this);

  wtfUi->setupUi(wtfDialog);
  connect(ui->wtf, SIGNAL(clicked()), wtfDialog, SLOT(show()));

  ui->motors_1->lock(true);
  ui->motors_2->lock(true);
  ui->motors_1->showGeneral(false);
  ui->motors_2->showGeneral(false);
  ui->motors_1->addMotor(component()->motors[Mono::Bragg1], true,true);
  ui->motors_1->addMotor(component()->motors[Mono::Z1], true,true);
  ui->motors_1->addMotor(component()->motors[Mono::Tilt1], true,true);
  ui->motors_1->addMotor(component()->motors[Mono::Bend1ob], true,true);
  ui->motors_1->addMotor(component()->motors[Mono::Bend1ib], true,true);
  ui->motors_1->addMotor(component()->motors[Mono::Xdist], true,true);
  ui->motors_2->addMotor(component()->motors[Mono::Bragg2], true,true);
  ui->motors_2->addMotor(component()->motors[Mono::Z2], true,true);
  ui->motors_2->addMotor(component()->motors[Mono::Tilt2], true,true);
  ui->motors_2->addMotor(component()->motors[Mono::Bend2ob], true,true);
  ui->motors_2->addMotor(component()->motors[Mono::Bend2ib], true,true);

  energySetter = new EnergySetRevert(ui->energy);
  ui->energy->setRange(Mono::energyRange.first, Mono::energyRange.second);


  QVBoxLayout *calibrateLayout = new QVBoxLayout(calibrateDialog);
  foreach (Mono::Motors motk, component()->motors.keys()) {
    QCheckBox * chbk = new QCheckBox(calibrateDialog);
    chbk->setText(component()->motors[motk]->getDescription());
    calibrateBoxes[motk] = chbk;
    calibrateLayout->addWidget(chbk);
  }
  QDialogButtonBox * calibrateButtons = new QDialogButtonBox(calibrateDialog);
  calibrateButtons->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
  connect(calibrateButtons, SIGNAL(accepted()), calibrateDialog, SLOT(accept()));
  connect(calibrateButtons, SIGNAL(rejected()), calibrateDialog, SLOT(reject()));
  calibrateLayout->addWidget(calibrateButtons);
  calibrateDialog->hide();

  const QString bendTT = wtfUi->textBrowser->toHtml();
  ui->bend1ib->setToolTip(bendTT);
  ui->bend1ob->setToolTip(bendTT);
  ui->bend2ib->setToolTip(bendTT);
  ui->bend2ob->setToolTip(bendTT);

  connect(component(), SIGNAL(connectionChanged(bool)), SLOT(updateConnection(bool)));
  connect(component(), SIGNAL(calibrationChanged(bool)), SLOT(updateCalibration()));
  connect(component(), SIGNAL(motionChanged(bool)), ui->stop, SLOT(setEnabled(bool)));
  connect(component(), SIGNAL(energyChanged(double)), SLOT(updateEnergy()));
  connect(component(), SIGNAL(dBraggChanged(double)), SLOT(updateDBragg()));
  connect(component(), SIGNAL(dXChanged(double)), SLOT(updateDX()));
  connect(component(), SIGNAL(zSeparationChanged(double)),
          ui->zSeparation, SLOT(setValue(double)));
  connect(component(), SIGNAL(dZChanged(double)), SLOT(updateDZ()));
  connect(component(), SIGNAL(tilt1Changed(double)), SLOT(updateTilt1()));
  connect(component(), SIGNAL(tilt2Changed(double)), SLOT(updateTilt2()));
  connect(component(), SIGNAL(bend1obChanged(double)), SLOT(updateBend1ob()));
  connect(component(), SIGNAL(bend2obChanged(double)), SLOT(updateBend2ob()));
  connect(component(), SIGNAL(bend1ibChanged(double)), SLOT(updateBend1ib()));
  connect(component(), SIGNAL(bend2ibChanged(double)), SLOT(updateBend2ib()));
  connect(component(), SIGNAL(inBeamChanged(Mono::InOutPosition)),
          SLOT(updateInOut(Mono::InOutPosition)));

  connect(component()->motors[Mono::Bragg2], SIGNAL(changedUserPosition(double)),
          SLOT(updateMotorBragg2()));
  connect(component()->motors[Mono::Bragg1], SIGNAL(changedUserPosition(double)),
          SLOT(updateMotorBragg1()));
  connect(component()->motors[Mono::Xdist], SIGNAL(changedUserPosition(double)),
          SLOT(updateMotorX()));
  connect(component()->motors[Mono::Bragg2], SIGNAL(changedMoving(bool)),
          SLOT(updateDBragg()));
  connect(component()->motors[Mono::Bragg1], SIGNAL(changedMoving(bool)),
          SLOT(updateDBragg()));
  connect(component()->motors[Mono::Bragg2], SIGNAL(changedMoving(bool)),
          SLOT(updateDX()));
  connect(component()->motors[Mono::Xdist], SIGNAL(changedMoving(bool)),
          SLOT(updateDX()));

  connect(component()->motors[Mono::Bragg1], SIGNAL(changedMoving(bool)),
          SLOT(updateEnergyMotion()));
  connect(component()->motors[Mono::Z2], SIGNAL(changedMoving(bool)),
          SLOT(updateEnergyMotion()));
  connect(component()->motors[Mono::Bragg2], SIGNAL(changedMoving(bool)),
          SLOT(updateEnergyMotion()));
  connect(component()->motors[Mono::Xdist], SIGNAL(changedMoving(bool)),
          SLOT(updateEnergyMotion()));
  connect(component()->motors[Mono::Z1], SIGNAL(changedMoving(bool)),
          ui->modeSetEnable, SLOT(setDisabled(bool)));
  connect(component()->motors[Mono::Tilt1],  SIGNAL(changedMoving(bool)),
          ui->tilt1, SLOT(setDisabled(bool)));
  connect(component()->motors[Mono::Tilt2],  SIGNAL(changedMoving(bool)),
          ui->tilt2, SLOT(setDisabled(bool)));
  connect(component()->motors[Mono::Bend1ib],  SIGNAL(changedMoving(bool)),
          ui->bend1ib, SLOT(setDisabled(bool)));
  connect(component()->motors[Mono::Bend1ob],  SIGNAL(changedMoving(bool)),
          ui->bend1ob, SLOT(setDisabled(bool)));
  connect(component()->motors[Mono::Bend2ib],  SIGNAL(changedMoving(bool)),
          ui->bend2ib, SLOT(setDisabled(bool)));
  connect(component()->motors[Mono::Bend2ob],  SIGNAL(changedMoving(bool)),
          ui->bend2ob, SLOT(setDisabled(bool)));

  foreach(QCaMotor * mot, component()->motors) {
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
  connect(ui->bend1ob, SIGNAL(valueEdited(double)), component(), SLOT(setBend1ob(double)));
  connect(ui->bend2ob, SIGNAL(valueEdited(double)), component(), SLOT(setBend2ob(double)));
  connect(ui->bend1ib, SIGNAL(valueEdited(double)), component(), SLOT(setBend1ib(double)));
  connect(ui->bend2ib, SIGNAL(valueEdited(double)), component(), SLOT(setBend2ib(double)));
  connect(ui->tilt1, SIGNAL(valueEdited(double)), component(), SLOT(setTilt1(double)));
  connect(ui->tilt2, SIGNAL(valueEdited(double)), component(), SLOT(setTilt2(double)));
  connect(ui->zSeparation, SIGNAL(valueEdited(double)), SLOT(onZseparationSet()));
  connect(ui->tuneZ, SIGNAL(valueEdited(double)), component(), SLOT(setDZ(double)));
  connect(ui->moveIn, SIGNAL(clicked()), component(), SLOT(moveIn()));
  connect(ui->moveOut, SIGNAL(clicked()), component(), SLOT(moveOut()));
  connect(ui->stop, SIGNAL(clicked()), component(), SLOT(stop()));
  connect(ui->calibrate, SIGNAL(clicked()), SLOT(onCalibration()));

  updateConnection(component()->isConnected());
  updateEnergy();
  updateMotorBragg1();
  updateMotorBragg2();
  updateMotorX();
  onAdvancedControl();

}

MonoGui::~MonoGui() {
    delete ui;
}


bool MonoGui::eventFilter(QObject *obj, QEvent *event) {
  if ( event->type() == QEvent::Resize) {
    if (ui->advancedWidget->isVisible()) {
      ui->label1->setMinimumWidth(ui->motors_1->width());
      ui->label2->setMinimumWidth(ui->motors_2->width());
    } else {
      ui->label1->setMinimumWidth(0);
      ui->label2->setMinimumWidth(0);
    }
  }
  return QObject::eventFilter(obj, event);
}



void MonoGui::updateConnection(bool con) {
  ui->mainWidget->setEnabled(component()->isConnected());
  ui->stop->setText( con ? "Stop all" : "No link");
  ui->stop->setStyleSheet( con ? "" : "color: rgb(255, 0, 0); background-color: rgb(0, 0, 0);");
  if (con) {
    ui->stop->setEnabled(component()->isMoving());
    ui->tuneBragg->setIncrement(component()->motors[Mono::Bragg1]->getStep());
    ui->tuneX->setIncrement(component()->motors[Mono::Xdist]->getStep());
    ui->tilt1->setIncrement(component()->motors[Mono::Tilt1]->getStep());
    ui->tilt2->setIncrement(component()->motors[Mono::Tilt2]->getStep());
    ui->bend1ob->setIncrement(component()->motors[Mono::Bend1ob]->getStep());
    ui->bend1ib->setIncrement(component()->motors[Mono::Bend1ib]->getStep());
    ui->bend2ob->setIncrement(component()->motors[Mono::Bend2ob]->getStep());
    ui->bend2ib->setIncrement(component()->motors[Mono::Bend2ib]->getStep());
    ui->tuneZ->setIncrement(component()->motors[Mono::Z2]->getStep());
  }
}

void MonoGui::updateCalibration() {
  if (component()->isCalibrated()) {
    ui->calibrate->setText("Calibrate motors");
    ui->calibrate->setStyleSheet("");
    ui->calibrate->setVisible(ui->advancedWidget->isVisible());
  } else {
    ui->calibrate->setText("WARNING! Motor(s) calibration may be lost. Click here to calibrate.");
    ui->calibrate->setStyleSheet("color: rgb(128, 0, 0);");
    ui->calibrate->setVisible(true);
  }
  foreach (Mono::Motors motk, component()->motors.keys())
    calibrateBoxes[motk]->setStyleSheet( component()->calibrated()[motk] ?
                                           "" : "color: rgb(128, 0, 0);" );
}


void MonoGui::updateEnergyMotion() {
  bool mov =
      component()->motors[Mono::Bragg1]->isMoving() ||
      component()->motors[Mono::Z2]->isMoving() ||
      component()->motors[Mono::Bragg2]->isMoving() ||
      component()->motors[Mono::Xdist]->isMoving();
  ui->energy->setDisabled(mov);
  ui->diffractions->setDisabled(mov);
  ui->lockBragg->setDisabled(mov);
  ui->lockDZ->setDisabled(mov);
  ui->lockX->setDisabled(mov);
  ui->tuneBragg->setDisabled(mov);
  ui->tuneX->setDisabled(mov);
  ui->tuneZ->setDisabled(mov);
  ui->zSeparationEnable->setDisabled(mov);
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

  if (component()->diffraction() == Mono::Si111) {
    ui->si111->setStyleSheet("font: bold;");
    ui->si311->setStyleSheet("");
  } else {
    ui->si111->setStyleSheet("");
    ui->si311->setStyleSheet("font: bold;");
  }

  if ( ui->energy->hasFocus() || ui->si111->hasFocus() || ui->si311->hasFocus() )
    return;

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
  case Mono::INBEAM :
      ui->currentInOut->setText("IN beam");
      ui->moveIn->setChecked(true);
      ui->moveIn->setStyleSheet("font: bold;");
      break;
  case Mono::OUTBEAM :
      ui->currentInOut->setText("OUT of the beam");
      ui->moveOut->setChecked(true);
      ui->moveOut->setStyleSheet("font: bold;");
      break;
  case Mono::BETWEEN :
    ui->currentInOut->setText(
          "between: " +
          QString::number(component()->motors[Mono::Z1]->getUserPosition(),'f', 2));
    ui->currentInOut->setStyleSheet(ui->status->styleSheet());
    ui->moveIn->setChecked(false);
    ui->moveOut->setChecked(false);
    break;
  case Mono::MOVING :
    ui->currentInOut->setText(
          "moving: " +
          QString::number(component()->motors[Mono::Z1]->getUserPosition(),'f', 2));
    ui->moveIn->setChecked(false);
    ui->moveOut->setChecked(false);
    break;
  }
  updateStatus();
}


void MonoGui::updateDBragg() {
  static const double expectedMaxDBragg = 1.0;
  ui->readDBragg->setText(QString::number(component()->dBragg(), 'f', ui->tuneBragg->decimals()));
  if ( ! component()->motors[Mono::Bragg1]->isMoving() &&
       ! component()->motors[Mono::Bragg2]->isMoving() )
    ui->tuneBragg->setValue(component()->dBragg());
  dBraggHealth =
      component()->motors[Mono::Bragg1]->isMoving() ||
      component()->motors[Mono::Bragg2]->isMoving() ||
      qAbs(component()->dBragg()) < expectedMaxDBragg;
  //ui->bigDBragg->setVisible( ! dBraggHealth );
  updateStatus();
}


void MonoGui::updateDX() {
  static const double expectedMaxDX = 0.01;
  ui->readDX->setText(QString::number(component()->dX(), 'f', ui->tuneX->decimals()));
  if ( ! component()->motors[Mono::Bragg2]->isMoving() &&
       ! component()->motors[Mono::Xdist]->isMoving() )
    ui->tuneX->setValue(component()->dX());
  dXhealth =
      component()->motors[Mono::Bragg2]->isMoving() ||
      component()->motors[Mono::Xdist]->isMoving()  ||
      qAbs(component()->dX()) < expectedMaxDX;
  //ui->bigDX->setVisible( ! dXhealth );
  updateStatus();
}


void MonoGui::updateDZ() {
  static const double expectedMaxDZ = 0.001;
  ui->readDZ->setText(QString::number(component()->zTweak(), 'f', ui->tuneZ->decimals()));
  if ( ! component()->motors[Mono::Z2]->isMoving() )
    ui->tuneZ->setValue(component()->zTweak());
  dZhealth =
      component()->motors[Mono::Z2]->isMoving() ||
      qAbs(component()->zTweak()) < expectedMaxDZ;
  //ui->bigDZ->setVisible( ! dZhealth );
  updateStatus();
}


void MonoGui::updateStatus() {
  ui->status->setVisible( component()->inBeam() == Mono::BETWEEN ||
                          ! dBraggHealth || ! dXhealth || ! dZhealth );
}

void MonoGui::updateTilt1() {
  ui->readTilt1->setText(QString::number(component()->tilt1(), 'f', ui->tilt1->decimals()));
  if ( ! component()->motors[Mono::Tilt1]->isMoving() )
    ui->tilt1->setValue(component()->tilt1());
}


void MonoGui::updateTilt2() {
  ui->readTilt2->setText(QString::number(component()->tilt2(), 'f', ui->tilt2->decimals()));
  if ( ! component()->motors[Mono::Tilt2]->isMoving() )
    ui->tilt2->setValue(component()->tilt2());
}


void MonoGui::updateBend1ob() {
  ui->readB1ob->setText(QString::number(component()->bend1ob(), 'f', ui->bend1ob->decimals()));
  if ( ! component()->motors[Mono::Bend1ob]->isMoving() )
    ui->bend1ob->setValue(component()->bend1ob());
}


void MonoGui::updateBend1ib() {
  ui->readB1ib->setText(QString::number(component()->bend1ib(), 'f', ui->bend1ib->decimals()));
  if ( ! component()->motors[Mono::Bend1ib]->isMoving() )
    ui->bend1ib->setValue(component()->bend1ib());
}

void MonoGui::updateBend2ob() {
  ui->readB2ob->setText(QString::number(component()->bend2ob(), 'f', ui->bend2ob->decimals()));
  if ( ! component()->motors[Mono::Bend2ob]->isMoving() )
    ui->bend2ob->setValue(component()->bend2ob());
}


void MonoGui::updateBend2ib() {
  ui->readB2ib->setText(QString::number(component()->bend2ib(), 'f', ui->bend2ib->decimals()));
  if ( ! component()->motors[Mono::Bend2ib]->isMoving() )
    ui->bend2ib->setValue(component()->bend2ib());
}



void MonoGui::onAdvancedControl() {
  if (ui->advancedWidget->isVisibleTo(this)) {
    ui->calibrate->setVisible(!component()->isCalibrated());
    ui->advancedWidget->setVisible(false);
    ui->advanced_pb->setText("Show advanced control");
    ui->advanced_pb->setStyleSheet("");
    ui->zSeparation->setEnabled(false);
    ui->modeSet->setEnabled(false);
  } else if ( PsswDial::ask(this) ) {
    ui->calibrate->setVisible(true);
    ui->advancedWidget->setVisible(true);
    ui->advanced_pb->setText("CLICK here to hide advanced control");
    ui->advanced_pb->setStyleSheet("background-color: rgba(255, 0, 0,64);");
    ui->zSeparation->setEnabled(true);
    ui->modeSet->setEnabled(true);
  }
}


void MonoGui::onCalibration() {
  foreach (Mono::Motors motk, component()->motors.keys()) {
    calibrateBoxes[motk]->setText(component()->motors[motk]->getDescription());
    calibrateBoxes[motk]->setChecked(!component()->calibrated()[motk]);
  }
  if ( ! PsswDial::ask(this) ||
       calibrateDialog->exec() != QDialog::Accepted )
    return;

  QList<Mono::Motors> mots;
  foreach (Mono::Motors motk, component()->motors.keys())
    if ( calibrateBoxes[motk]->isChecked() )
      mots << motk;
  if ( ! mots.isEmpty() )
    component()->calibrate(mots);

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
  if (mot == component()->motors[Mono::Xdist]) lab = ui->readDX;
  else if (mot == component()->motors[Mono::Bragg1]) lab = ui->readBragg1;
  else if (mot == component()->motors[Mono::Bragg2]) lab = ui->readBragg2;
  else if (mot == component()->motors[Mono::Tilt1]) lab = ui->readTilt1;
  else if (mot == component()->motors[Mono::Tilt2]) lab = ui->readTilt2;
  else if (mot == component()->motors[Mono::Z1]) lab = ui->currentInOut;
  else if (mot == component()->motors[Mono::Z2]) lab = ui->readDZ;
  else if (mot == component()->motors[Mono::Bend1ob]) lab = ui->readB1ob;
  else if (mot == component()->motors[Mono::Bend1ib]) lab = ui->readB1ib;
  else if (mot == component()->motors[Mono::Bend2ob]) lab = ui->readB2ob;
  else if (mot == component()->motors[Mono::Bend2ib]) lab = ui->readB2ib;
  else return;
  lab->setStyleSheet(
        (mot->getHiLimitStatus() || mot->getLoLimitStatus()) ?
          "background-color: rgb(128, 0, 0); color: rgb(255, 255, 255);" :
          "");
  }






