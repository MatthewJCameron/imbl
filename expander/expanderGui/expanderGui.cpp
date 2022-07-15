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
  connect(component(), SIGNAL(inBeamChanged(Expander::InOutPosition)),       SLOT(updateInOut(Expander::InOutPosition)));

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->moveIn, SIGNAL(clicked()), component(), SLOT(moveIn()));
  connect(ui->moveOut, SIGNAL(clicked()), component(), SLOT(moveOut()));
  connect(ui->stop, SIGNAL(clicked()), component(), SLOT(stop()));
  
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


void ExpanderGui::updateInOut(Expander::InOutPosition iopos) {
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


//void ExpanderGui::updateStatus() {
//  ui->status->setVisible( component()->inBeam() == expander::BETWEEN);
//}

void ExpanderGui::onAdvancedControl() {
  if (ui->advancedWidget->isVisibleTo(this)) {
    ui->advancedWidget->setVisible(false);
    ui->advanced_pb->setText("Show advanced control");
    ui->advanced_pb->setStyleSheet("");
  } else if ( PsswDial::ask(this) ) {
    ui->advancedWidget->setVisible(true);
    ui->advanced_pb->setText("CLICK here to hide advanced control");
    ui->advanced_pb->setStyleSheet("background-color: rgba(255, 0, 0,64);");
  }
}