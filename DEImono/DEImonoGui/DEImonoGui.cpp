#include "DEImonoGui.h"
#include "ui_DEImonoGui.h"
#include "error.h"

const double DEImonoGui::plankevXlighspeed = 12.398419; // h*c (keV)

DEImonoGui::DEImonoGui(QWidget *parent) :
  ComponentGui(new DEImono(parent), true, parent),
  ui(new Ui::DEImonoGui)
{
  init();
}

DEImonoGui::DEImonoGui(DEImono * mono, QWidget *parent) :
  ComponentGui(mono, true, parent),
  ui(new Ui::DEImonoGui)
{
  init();
}


void DEImonoGui::init() {

  ui->setupUi(this);
  ui->advanced->hide();
  ui->calibrationWidget->hide();

  ui->motors->lock(true);
  ui->motors->addMotor(DEImono::usMotor,true,true);
  ui->motors->addMotor(DEImono::dsMotor,true,true);
  ui->motors->addMotor(DEImono::zMotor,true,true);

  connect(component(), SIGNAL(energyChanged(double)), ui->energy, SLOT(setValue(double)));
  connect(component(), SIGNAL(motionChanged(bool)), SLOT(updateMotion(bool)));
  connect(component(), SIGNAL(dsDisplacementChanged(double)), ui->displacementDS, SLOT(setValue(double)));
  connect(component(), SIGNAL(zDisplacementChanged(double)), ui->displacementZ, SLOT(setValue(double)));

  connect(ui->energy, SIGNAL(valueChanged(double)), SLOT(onEnergyChange()));
  connect(ui->energy, SIGNAL(editingFinished()), SLOT(onEnergySet()));
  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->showCalibration, SIGNAL(clicked()), SLOT(onShowCalibration()));
  connect(ui->materials, SIGNAL(activated(int)), SLOT(onChangingMaterial()));
  connect(ui->calibrate, SIGNAL(clicked()), SLOT(onCalibrate()));

  connect(ui->displacementDS, SIGNAL(editingFinished()), SLOT(onTweakDS()));
  connect(ui->tweakNegDS, SIGNAL(clicked()), SLOT(onTweakDS()));
  connect(ui->tweakPosDS, SIGNAL(clicked()), SLOT(onTweakDS()));

  connect(ui->displacementZ, SIGNAL(editingFinished()), SLOT(onTweakZ()));
  connect(ui->tweakNegZ, SIGNAL(clicked()), SLOT(onTweakZ()));
  connect(ui->tweakPosZ, SIGNAL(clicked()), SLOT(onTweakZ()));

  updateConnection(component()->isConnected());

}

DEImonoGui::~DEImonoGui()
{
    delete ui;
}



void DEImonoGui::updateMotion(bool moving) {
  ui->stop->setVisible(moving);
  ui->mainWdg->setEnabled(!moving);
}

void DEImonoGui::updateConnection(bool con) {
  ui->calibrate->setEnabled(con);
  ui->mainWdg->setEnabled(con);
  ui->stop->setEnabled(con);
  ui->stop->setText( con ? "Stop" : "Disconnected");
  if (con) {
    updateMotion(component()->isMoving());
    ui->energy->setValue(component()->energy());
    ui->displacementDS->setValue(component()->dsDisplacement());
    ui->displacementZ->setValue(component()->zDisplacement());
  }
}


void DEImonoGui::onAdvancedControl() {
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

void DEImonoGui::onShowCalibration() {
  if ( ui->calibrationWidget->isVisible() ) {
    ui->calibrationWidget->setVisible(false);
    ui->showCalibration->setText("Calibrate");
  } else if ( PsswDial::ask(this) ) {
    ui->calibrationWidget->setVisible(true);
    ui->showCalibration->setText("Hide calibration");
  }
}

void DEImonoGui::onChangingMaterial() {
  const QString & matName = ui->materials->currentText();
  double kEdge;
  if      (matName == "Zr") kEdge = 17.998 ;
  else if (matName == "Mo") kEdge = 20.000 ;
  else if (matName == "Ag") kEdge = 25.514 ;
  else if (matName == "Sn") kEdge = 29.200 ;
  else if (matName == "Ta") kEdge = 67.416 ;
  else if (matName == "Ir") kEdge = 76.111 ;
  else if (matName == "Au") kEdge = 80.726 ;
  else if (matName == "Pb") kEdge = 88.005 ;
  else {
    warn("Unknown material \""+matName+"\" for calibration");
    return;
  }
  ui->calibrationEnergy->setValue(kEdge);
}

void DEImonoGui::onCalibrate() {
  double energy = ui->calibrationEnergy->value();
  DEImono::usMotor->setUserPosition(DEImono::usPosition(energy));
  DEImono::dsMotor->setUserPosition(DEImono::dsPosition(energy));
  DEImono::zMotor->setUserPosition(DEImono::zPosition(energy));
  qDebug() << DEImono::usPosition(energy) << DEImono::dsPosition(energy) << DEImono::zPosition(energy);
  onShowCalibration();
}

void DEImonoGui::onEnergyChange() {

  double energy = ui->energy->value();
  if ( energy <= DEImono::totalCoeff) { // Should never happen
    warn( "Can't calculate crystal position for energy " + QString::number(energy) + "kEv.", this);
    return;
  }

  ui->calibrationEnergy->setValue(energy);
  ui->wave->setValue( plankevXlighspeed / energy );
  double thetaB = asin( DEImono::totalCoeff / energy );
  ui->angle->setValue( thetaB * 180.0 / M_PI ); // rad -> deg

}

void DEImonoGui::onEnergySet() {
  component()->setEnergy(ui->energy->value(),
                         ui->lockDisplacementDS->isChecked(),
                         ui->lockDisplacementZ->isChecked() );
}

void DEImonoGui::onTweakDS() {
  QObject * sendr = sender();
  if (sendr == ui->tweakNegDS)
    ui->displacementDS->setValue( ui->displacementDS->value() - ui->tweakDS->value() );
  else if (sendr == ui->tweakPosDS)
    ui->displacementDS->setValue( ui->displacementDS->value() + ui->tweakDS->value() );
  component()->setDSdisplacement(ui->displacementDS->value());
}

void DEImonoGui::onTweakZ() {
  QObject * sendr = sender();
  if (sendr == ui->tweakNegZ)
    ui->displacementZ->setValue( ui->displacementZ->value() - ui->tweakZ->value() );
  else if (sendr == ui->tweakPosZ)
    ui->displacementZ->setValue( ui->displacementZ->value() + ui->tweakZ->value() );
  component()->setZdisplacement(ui->displacementZ->value());
}

