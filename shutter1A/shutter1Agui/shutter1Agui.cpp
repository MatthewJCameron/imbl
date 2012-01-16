#include "shutter1Agui.h"
#include "ui_shutter1Agui.h"
#include <QDebug>


const QString Shutter1Agui::styleTemplate =
"background-color: qradialgradient("
"spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,"
" stop:%1 rgba(255, 255, 255, 255), stop:%2 rgba(%3));";


Shutter1Agui::Shutter1Agui(QWidget *parent) :
  ComponentGui(new Shutter1A(parent), true, parent),
  ui(new Ui::Shutter1Agui)
{
  ui->setupUi(this);
  connect(ui->open, SIGNAL(clicked()), component(), SLOT(open()));
  connect(ui->close, SIGNAL(clicked()), component(), SLOT(close()));
  connect(component(), SIGNAL(stateChanged(State)), SLOT(updateStatuses()));
  connect(component(), SIGNAL(relaxChanged()), SLOT(updateStatuses()));
  updateConnection(false);
}

Shutter1Agui::Shutter1Agui(Shutter1A * sht, QWidget *parent) :
  ComponentGui(sht, false, parent),
  ui(new Ui::Shutter1Agui)
{
  ui->setupUi(this);
  connect(ui->open, SIGNAL(clicked()), component(), SLOT(open()));
  connect(ui->close, SIGNAL(clicked()), component(), SLOT(close()));
  connect(component(), SIGNAL(stateChanged(State)), SLOT(updateStatuses()));
  connect(component(), SIGNAL(relaxChanged()), SLOT(updateStatuses()));
  connect(component(), SIGNAL(enabledChanged()), SLOT(updateStatuses()));
  updateConnection(component()->isConnected());
}

Shutter1Agui::~Shutter1Agui() {
  delete ui;
}

void Shutter1Agui::updateConnection(bool con) {
  if ( !con )
    ui->label->setText("Disconnected");
  else
    updateStatuses();
}

void Shutter1Agui::updateStatuses() {
  switch ( component()->state() ) {
  case Shutter1A::OPENED:
    ui->open->setStyleSheet( styleTemplate.arg("0.0", "1.0", "128, 0, 0, 200") );
    ui->close->setStyleSheet( styleTemplate.arg("0.0", "0.5", "0, 128, 0, 255") );
    setStyleSheet( "background-color: rgba(128, 0, 0, 200);" );
    break;
  case Shutter1A::CLOSED:
    ui->open->setStyleSheet( styleTemplate.arg("0.0", "0.5", "128, 0, 0, 255") );
    ui->close->setStyleSheet( styleTemplate.arg("0.0", "1.0", "0, 128, 0, 200") );
    setStyleSheet( "background-color: rgba(0, 128, 0, 200);" );
    break;
  case Shutter1A::BETWEEN:
    ui->open->setStyleSheet( styleTemplate.arg("0.0", "0.5", "128, 0, 0, 200") );
    ui->close->setStyleSheet( styleTemplate.arg("0.0", "0.5", "0, 128, 0, 200") );
    setStyleSheet( "background-color: rgba(128, 128, 0, 200);" );
    break;
  }
  ui->open->setEnabled( ! component()->isRelaxing() && component()->isEnabled() );
  ui->close->setEnabled( ! component()->isRelaxing() && component()->isEnabled() );
  ui->label->setText( component()->objectName() + ":\n"
                      + component()->description() + "\n"
                      + (component()->isEnabled() ? "En" : "Dis") + "abled\n"
                      + (component()->isRelaxing() ? "Relaxing" : "") );
}




