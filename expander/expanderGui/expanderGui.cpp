#include "expanderGui.h"
#include "error.h"
#include "ui_expanderGui.h"

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
  ui->motors_1->addMotor(component()->motors[Expander::tbly], true,true);
  ui->motors_1->addMotor(component()->motors[Expander::tblz], true,true);
  
  connect(component(), SIGNAL(connectionChanged(bool)), SLOT(updateConnection(bool)));
  connect(component(), SIGNAL(motionChanged(bool)), ui->stop, SLOT(setEnabled(bool)));
  connect(component(), SIGNAL(expInBeamChanged(Expander::InOutPosition)),
       SLOT(updateExpInOut(Expander::InOutPosition)));
  connect(component(), SIGNAL(tblInBeamChanged(Expander::InOutPosition)),
       SLOT(updateTblInOut(Expander::InOutPosition)));

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->moveIn, SIGNAL(clicked()), component(), SLOT(expIn()));
  connect(ui->moveOut, SIGNAL(clicked()), component(), SLOT(expOut()));
  connect(ui->stop, SIGNAL(clicked()), component(), SLOT(stop()));
  
  connect(ui->tblIn, SIGNAL(clicked()), component(), SLOT(tblIn()));
  connect(ui->tblOut, SIGNAL(clicked()), component(), SLOT(tblOut()));

  connect(component()->motors[Expander::inOut], SIGNAL(changedMoving(bool)), ui->modeSetEnable, SLOT(setDisabled(bool)));
  connect(component()->motors[Expander::tblz], SIGNAL(changedMoving(bool)), ui->modeTblSetEnable, SLOT(setDisabled(bool)));

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
      ui->currentTblInOut->setText("IN beam");
      ui->tblIn->setFlat(true);
      ui->tblIn->setStyleSheet("font: bold;");
      break;
  case Expander::OUTBEAM :
      ui->currentInOut->setText("OUT of the beam");
      ui->tblOut->setFlat(true);
      ui->tblOut->setStyleSheet("font: bold;");
      break;
  case Expander::BETWEEN :
    //ui->currentInOut->setStyleSheet(ui->status->styleSheet());
    break;
  case Expander::MOVING :
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
    //ui->modeSet->setEnabled(true);
    //ui->tblSet->setEnabled(true);
  }
}