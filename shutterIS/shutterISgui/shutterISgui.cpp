#include "shutterISgui.h"
#include "ui_shutterISgui.h"
#include <QDebug>


const QString ShutterISgui::styleTemplate =
"background-color: qradialgradient("
"spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,"
" stop:%1 rgba(255, 255, 255, 255), stop:%2 rgba(%3));";


ShutterISgui::ShutterISgui(QWidget *parent) :
  ComponentGui(new ShutterIS(parent), true, parent),
  ui(new Ui::ShutterISgui)
{
  ui->setupUi(this);
  connect(ui->open, SIGNAL(clicked()), component(), SLOT(open()));
  connect(ui->close, SIGNAL(clicked()), component(), SLOT(close()));
  connect(component(), SIGNAL(stateChanged(ShutterIS::State)), SLOT(updateStatuses()));
  updateConnection(false);
}

ShutterISgui::ShutterISgui(ShutterIS * sht, QWidget *parent) :
  ComponentGui(sht, false, parent),
  ui(new Ui::ShutterISgui)
{
  ui->setupUi(this);
  connect(ui->open, SIGNAL(clicked()), component(), SLOT(open()));
  connect(ui->close, SIGNAL(clicked()), component(), SLOT(close()));
  connect(component(), SIGNAL(stateChanged(State)), SLOT(updateStatuses()));
  connect(component(), SIGNAL(relaxChanged()), SLOT(updateStatuses()));
  connect(component(), SIGNAL(enabledChanged()), SLOT(updateStatuses()));
  updateConnection(component()->isConnected());
}

ShutterISgui::~ShutterISgui() {
  delete ui;
}

void ShutterISgui::updateConnection(bool con) {
  if ( !con ) {
    ui->isLabel->setText("Disconnected");
  } else
    updateStatuses();
}

void ShutterISgui::updateState() {
  switch ( component()->state() ) {
    case ShutterIS::OPENED:
      ui->open->setStyleSheet( styleTemplate.arg("0.0", "1.0", "128, 0, 0, 200") );
      ui->close->setStyleSheet( styleTemplate.arg("0.0", "0.5", "0, 128, 0, 255") );
      setStyleSheet( "background-color: rgba(128, 0, 0, 200);" );
      break;
    case ShutterIS::CLOSED:
      ui->open->setStyleSheet( styleTemplate.arg("0.0", "0.5", "128, 0, 0, 255") );
      ui->close->setStyleSheet( styleTemplate.arg("0.0", "1.0", "0, 128, 0, 200") );
      setStyleSheet( "background-color: rgba(0, 128, 0, 200);" );
      break;
    case ShutterIS::BETWEEN:
      ui->open->setStyleSheet( styleTemplate.arg("0.0", "0.5", "128, 0, 0, 200") );
      ui->close->setStyleSheet( styleTemplate.arg("0.0", "0.5", "0, 128, 0, 200") );
      setStyleSheet( "background-color: rgba(128, 128, 0, 200);" );
      break;
  }
  ui->open->setEnabled( ! component()->isRelaxing() && component()->isEnabled() );
  ui->close->setEnabled( ! component()->isRelaxing() && component()->isEnabled() );
  ui->isLabel->setText( component()->objectName() + ":\n"
                      + component()->description()
                      + (component()->isRelaxing() ? " / Relaxing" : "") );

}




