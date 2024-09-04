#include "expanderGui.h"
#include "error.h"
#include "ui_expanderGui.h"
#include "mono/monoGui/ui_energysetter.h"
#include "tuner.h"

EnergySetRevertGonio::EnergySetRevertGonio(QWidget * master) :
  QWidget(static_cast<QWidget*>(master->parent())),
  ui(new Ui::EnergySetter)
{
  ui->setupUi(this);
  connect(ui->set, SIGNAL(clicked()), SIGNAL(set()));
  connect(ui->revert, SIGNAL(clicked()), SIGNAL(revert()));
  master->installEventFilter(this);
}

bool EnergySetRevertGonio::eventFilter(QObject *obj, QEvent *event) {
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
    }
  }
  return QObject::eventFilter(obj, event);
}

bool EnterEscapePressEaterGonio::eventFilter(QObject * obj, QEvent * event) {
  bool ret = QObject::eventFilter(obj, event);
  if ( event->type() == QEvent::KeyPress ) {
    int key = static_cast<QKeyEvent*>(event)->key();
    if ( key == Qt::Key_Enter || key == Qt::Key_Return )
      emit enterPressed();
    else if ( key == Qt::Key_Escape )
      emit escapePressed();
  }
  return ret;
};


ExpanderGui::ExpanderGui(QWidget *parent) :
  ComponentGui(new Expander(parent), true, parent),
  ui(new Ui::ExpanderGui)
{
  init();
}

ExpanderGui::ExpanderGui(Expander * expander, QWidget *parent) :
  ComponentGui(expander, true, parent),
  ui(new Ui::ExpanderGui)
{
  init();
}


void ExpanderGui::init() {
  ui->setupUi(this);
  //ui->motors_1->installEventFilter(this);
  ui->motors_1->lock(true);
  ui->motors_1->showGeneral(false);
  ui->motors_1->addMotor(component()->motors[Expander::tilt], true,true);
  ui->motors_1->addMotor(component()->motors[Expander::slide], true,true);
  ui->motors_1->addMotor(component()->motors[Expander::gonio], true,true);
  ui->motors_1->addMotor(component()->motors[Expander::inOut], true,true);
  //ui->motors_1->addMotor(component()->motors[Expander::tbly], true,true);
  ui->motors_1->addMotor(component()->motors[Expander::tblz], true,true);

  energySetter = new EnergySetRevertGonio(ui->energy);
  ui->energy->setRange(Expander::energyRange.first, Expander::energyRange.second);

  EnterEscapePressEaterGonio * eePress = new EnterEscapePressEaterGonio(this);
  ui->energy->installEventFilter(eePress);



  connect(component(), SIGNAL(connectionChanged(bool)), SLOT(updateConnection(bool)));
  connect(component(), SIGNAL(motionChanged(bool)), ui->stop, SLOT(setEnabled(bool)));
  connect(component(), SIGNAL(expInBeamChanged(Expander::InOutPosition)), SLOT(updateExpInOut(Expander::InOutPosition)));
  connect(component(), SIGNAL(tblInBeamChanged(Expander::InOutPosition)), SLOT(updateTblInOut(Expander::InOutPosition)));

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->moveIn, SIGNAL(clicked()), component(), SLOT(expIn()));
  connect(ui->moveOut, SIGNAL(clicked()), component(), SLOT(expOut()));
  connect(ui->stop, SIGNAL(clicked()), component(), SLOT(stop()));
  
  connect(ui->tblIn, SIGNAL(clicked()), component(), SLOT(tblIn()));
  connect(ui->tblOut, SIGNAL(clicked()), component(), SLOT(tblOut()));

  connect(component()->motors[Expander::inOut], SIGNAL(changedMoving(bool)), ui->modeSetEnable, SLOT(setDisabled(bool)));
  connect(component()->motors[Expander::tblz], SIGNAL(changedMoving(bool)), ui->modeTblSetEnable, SLOT(setDisabled(bool)));


  connect(component(), SIGNAL(energyChanged(double)), SLOT(updateEnergy()));
  connect(component(), SIGNAL(dBraggChanged(double)), SLOT(updateDBragg()));
  connect(ui->energy, SIGNAL(valueChanged(double)),  SLOT(onEnergyTune()));
  connect(energySetter, SIGNAL(set()),  SLOT(onEnergySet()));
  connect(energySetter, SIGNAL(revert()),  SLOT(revertEnergy()));
  connect(eePress, SIGNAL(enterPressed()),  SLOT(onEnergySet()));
  connect(eePress, SIGNAL(escapePressed()),  SLOT(revertEnergy()));
  connect(ui->tuneBragg, SIGNAL(valueEdited(double)), component(), SLOT(setDBragg(double)));
  connect(component()->motors[Expander::gonio], SIGNAL(changedMoving(bool)), SLOT(updateDBragg()));
  connect(component()->motors[Expander::gonio], SIGNAL(changedMoving(bool)), SLOT(updateEnergyMotion()));

connect(ui->lockBragg,SIGNAL(stateChanged(int)),component(),SLOT(setUseDBragg(int)));

  updateConnection(component()->isConnected());
  onAdvancedControl();

}

ExpanderGui::~ExpanderGui() {
    delete ui;
}


void ExpanderGui::updateConnection(bool con) {
  ui->mainWidget->setEnabled(component()->isConnected());
  ui->stop->setText( con ? "Stop all" : "No link");
  ui->stop->setStyleSheet( con ? "" : "color: rgb(255, 0, 0); background-color: rgb(0, 0, 0);");
  if (con) {
    ui->stop->setEnabled(component()->isMoving());
  }
}


void ExpanderGui::updateExpInOut(Expander::InOutPosition iopos) {
  ui->currentInOut->setStyleSheet("");
  ui->moveIn->setFlat(false);
  ui->moveOut->setFlat(false);
  ui->moveIn->setStyleSheet("");
  ui->moveOut->setStyleSheet("");
  ui->currentInOut->setText(QString::number(component()->motors[Expander::inOut]->getUserPosition(),'f', 2));
  switch (iopos) {
  case Expander::INBEAM :
      ui->currentInOut->setText("IN beam");
      ui->moveIn->setFlat(true);
      ui->moveIn->setStyleSheet("font: bold;");
      break;
  case Expander::OUTBEAM :
      ui->currentInOut->setText("OUT of the beam");
      ui->moveOut->setFlat(true);
      ui->moveOut->setStyleSheet("font: bold;");
      break;
  case Expander::BETWEEN :
    //ui->currentInOut->setStyleSheet(ui->status->styleSheet());
    break;
  case Expander::MOVING :
    break;
  }
  //updateStatus();
}

void ExpanderGui::updateTblInOut(Expander::InOutPosition iopos) {
  ui->currentTblInOut->setStyleSheet("");
  ui->tblIn->setFlat(false);
  ui->tblOut->setFlat(false);
  ui->tblIn->setStyleSheet("");
  ui->tblOut->setStyleSheet("");
  ui->currentTblInOut->setText(QString::number(component()->motors[Expander::tblz]->getUserPosition(),'f', 2));
  switch (iopos) {
  case Expander::INBEAM :
      //warn("Z INBEAM", objectName());
      ui->currentTblInOut->setText("IN beam");
      ui->tblIn->setFlat(true);
      ui->tblIn->setStyleSheet("font: bold;");
      break;
  case Expander::OUTBEAM :
      //warn("Z OUTBEAM", objectName());
      ui->currentTblInOut->setText("OUT of the beam");
      ui->tblOut->setFlat(true);
      ui->tblOut->setStyleSheet("font: bold;");
      break;
  case Expander::BETWEEN :
    //warn("Z BETWEEN", objectName());
    //ui->currentTblInOut->setStyleSheet(ui->status->styleSheet());
    break;
  case Expander::MOVING :
    //warn("Z MOVING", objectName());
    break;
  }
  //updateStatus();
}

//void ExpanderGui::updateStatus() {
//  ui->status->setVisible( component()->inBeam() == expander::BETWEEN);
//}

void ExpanderGui::onAdvancedControl() {
  if (ui->advancedWidget->isVisibleTo(this)) {
    ui->advancedWidget->setVisible(false);
    ui->advanced_pb->setText("Show advanced control");
    ui->advanced_pb->setStyleSheet("");
    ui->modeSet->setEnabled(false);
    ui->tblSet->setEnabled(false);
  } else if ( PsswDial::ask(this) ) {
    ui->advancedWidget->setVisible(true);
    ui->advanced_pb->setText("CLICK here to hide advanced control");
    ui->advanced_pb->setStyleSheet("background-color: rgba(255, 0, 0,64);");
    ui->modeSet->setEnabled(true);
    ui->tblSet->setEnabled(true);
  }
}

void ExpanderGui::updateEnergyMotion() {
  bool mov = component()->motors[Expander::gonio]->isMoving(); 
  ui->energy->setDisabled(mov);
  ui->lockBragg->setDisabled(mov);
  ui->tuneBragg->onMotionChange(mov);
}

void ExpanderGui::onEnergyTune() {
  const double enrg = ui->energy->value();
  bool setrevert =
      ! component()->motors[Expander::gonio]->isMoving() &&
      ( qAbs(enrg - component()->energy()) >= 0.001 ) ;
  energySetter->setVisible(setrevert);
}

void ExpanderGui::onEnergySet() {
  energySetter->hide();
  component()->setEnergy(ui->energy->value(),ui->lockBragg->isChecked());
  revertEnergy();
}

void ExpanderGui::revertEnergy() {
  if ( qAbs(ui->energy->value() - component()->energy()) >= 1.0e-03 )
    ui->energy->setValue(component()->energy());
  energySetter->hide();
}

void ExpanderGui::updateDBragg() {
  ui->readDBragg->setText(QString::number(component()->dBragg(), 'f', ui->tuneBragg->decimals()));
  if ( ! component()->motors[Expander::gonio]->isMoving() )
    ui->tuneBragg->setValue(component()->dBragg());
}

void ExpanderGui::updateEnergy() {
  if ( ui->energy->hasFocus() )
    return;
  else
    revertEnergy();
}
