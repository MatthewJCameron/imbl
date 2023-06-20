#include "shutterIAPgui.h"
#include "ui_shutterIAPgui.h"
#include <QDebug>


const QString ShutterIAPgui::styleTemplate =
"background-color: qradialgradient("
"spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,"
" stop:%1 rgba(255, 255, 255, 255), stop:%2 rgba(%3));";


ShutterIAPgui::ShutterIAPgui(QWidget *parent) :
  ComponentGui(new ShutterIAP(parent), true, parent),
  ui(new Ui::ShutterIAPgui)
{
  ui->setupUi(this);
  connect(ui->open, SIGNAL(clicked()), component(), SLOT(open()));
  connect(ui->close, SIGNAL(clicked()), component(), SLOT(close()));
  connect(component(), SIGNAL(stateChanged(ShutterIAP::State)), SLOT(updateState()));
  updateConnection(false);
}

ShutterIAPgui::ShutterIAPgui(ShutterIAP * sht, QWidget *parent) :
  ComponentGui(sht, false, parent),
  ui(new Ui::ShutterIAPgui)
{
  ui->setupUi(this);
  connect(ui->open, SIGNAL(clicked()), component(), SLOT(open()));
  connect(ui->close, SIGNAL(clicked()), component(), SLOT(close()));
  connect(component(), SIGNAL(stateChanged(State)), SLOT(updateState()));
  updateConnection(component()->isConnected());
}

ShutterIAPgui::~ShutterIAPgui() {
  delete ui;
}

void ShutterIAPgui::updateConnection(bool con) {
  if ( !con ) {
    ui->isLabel->setText("Disconnected");
  } else
    updateState();
}

void ShutterIAPgui::updateState() {
  switch ( component()->state() ) {
    case ShutterIAP::OPENED:
      ui->open->setStyleSheet( styleTemplate.arg("0.0", "1.0", "128, 0, 0, 200") );
      ui->close->setStyleSheet( styleTemplate.arg("0.0", "0.5", "0, 128, 0, 255") );
      setStyleSheet( "background-color: rgba(128, 0, 0, 200);" );
      break;
    case ShutterIAP::CLOSED:
      ui->open->setStyleSheet( styleTemplate.arg("0.0", "0.5", "128, 0, 0, 255") );
      ui->close->setStyleSheet( styleTemplate.arg("0.0", "1.0", "0, 128, 0, 200") );
      setStyleSheet( "background-color: rgba(0, 128, 0, 200);" );
      break;
    case ShutterIAP::BETWEEN:
      ui->open->setStyleSheet( styleTemplate.arg("0.0", "0.5", "128, 0, 0, 200") );
      ui->close->setStyleSheet( styleTemplate.arg("0.0", "0.5", "0, 128, 0, 200") );
      setStyleSheet( "background-color: rgba(128, 128, 0, 200);" );
      break;
  }
  ui->open->setEnabled(true);
  ui->close->setEnabled(true);
  //ui->isLabel->setText( component()->objectName() + ":\n" + component()->description() );
}




