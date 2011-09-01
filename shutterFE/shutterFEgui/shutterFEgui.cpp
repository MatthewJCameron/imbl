#include "shutterFEgui.h"
#include "ui_shutterFEgui.h"
#include <QDebug>

const QString ShutterFEgui::styleTemplate =
"background-color: qradialgradient("
"spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,"
" stop:%1 rgba(255, 255, 255, 255), stop:%2 rgba(%3));";


ShutterFEgui::ShutterFEgui(QWidget *parent) :
  ComponentGui(new ShutterFE(parent), true, parent),
  ui(new Ui::ShutterFEgui)
{
  ui->setupUi(this);
  connect(ui->open, SIGNAL(clicked()), component(), SLOT(open()));
  connect(ui->close, SIGNAL(clicked()), component(), SLOT(close()));
  connect(component(), SIGNAL(stateChanged(State)), SLOT(updateState()));
  connect(component(), SIGNAL(relaxChanged()), SLOT(updateRelax()));
  updateConnection(component()->isConnected());
}

ShutterFEgui::ShutterFEgui(ShutterFE * sht, QWidget *parent) :
  ComponentGui(sht, false, parent),
  ui(new Ui::ShutterFEgui)
{
  ui->setupUi(this);
  connect(ui->open, SIGNAL(clicked()), component(), SLOT(open()));
  connect(ui->close, SIGNAL(clicked()), component(), SLOT(close()));
  connect(component(), SIGNAL(stateChanged(State)), SLOT(updateState()));
  connect(component(), SIGNAL(relaxChanged()), SLOT(updateRelax()));
  updateConnection(component()->isConnected());
}

ShutterFEgui::~ShutterFEgui() {
  delete ui;
}

void ShutterFEgui::updateConnection(bool con) {
  setEnabled(con);
  if ( !con )
    ui->label->setText("Disconnected");
  else {
    updateState();
    updateRelax();
  }

}

void ShutterFEgui::updateState() {
  switch ( component()->state() ) {
  case ShutterFE::OPENED :
    ui->open->setStyleSheet( styleTemplate.arg("0.0", "1.0", "128, 0, 0, 200") );
    ui->close->setStyleSheet( styleTemplate.arg("0.0", "0.5", "0, 128, 0, 255") );
    setStyleSheet( "background-color: rgba(128, 0, 0, 200);" );
    break;
  case ShutterFE::CLOSED:
    ui->open->setStyleSheet( styleTemplate.arg("0.0", "0.5", "128, 0, 0, 255") );
    ui->close->setStyleSheet( styleTemplate.arg("0.0", "1.0", "0, 128, 0, 200") );
    setStyleSheet( "background-color: rgba(0, 128, 0, 200);" );
    break;
  case ShutterFE::BETWEEN:
    break;
  }
  ui->label->setText( component()->objectName() + ":\n"
                      + component()->description()
                      + (component()->isRelaxing() ? " / Relaxing" : "") );
}

void ShutterFEgui::updateRelax() {
  ui->open->setEnabled( ! component()->isRelaxing() );
  ui->close->setEnabled( ! component()->isRelaxing() );
  updateState();
}

