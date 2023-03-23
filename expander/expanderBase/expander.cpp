#include "expander.h"
#include "error.h"
#include <qtpv.h>
#include "shutterFE.h"
#include "shutter1A.h"
#include <QMessageBox>
#include <stdio.h>
const QString Expander::pvBaseName = "SR08ID01EXP01:";
const QString Expander::pvTableBaseName = "SR08ID01TBL13:";
const QHash<Expander::Motors,QCaMotor*> Expander::motors=Expander::init_motors();
const QPair<double,double> Expander::energyRange = qMakePair<double,double>(30.0,39.0);
const double Expander::theGradient = 0.791; 
//const double Expander::theGradient = -0.285912891; 
//const double Expander::theIntercept = 19.90119386;
//const double Expander::theIntercept = 11.46519;
const double Expander::theIntercept = -14.562;
const double expanderInPosition = 176.0;
const double expanderOutPosition = 18.0;
const double tableInPosition = 200.0;
const double tableOutPosition = 0.0;

Expander::Expander(QObject *parent) :
  Component("Expander", parent),
  iAmMoving(false),
  //_inBeam(BETWEEN),
  _tblInBeam(BETWEEN),
  _expInBeam(BETWEEN),
  _energy(0.0),
  _dBragg(0.0),
  _useDBragg(false)
{
  foreach(QCaMotor * mot, motors) {
    connect(mot, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
    connect(mot, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
  }
  connect(motors[inOut], SIGNAL(changedMoving(bool)), SLOT(UpdateExpInOutStatus()));
  connect(motors[inOut], SIGNAL(changedUserPosition(double)), SLOT(UpdateExpInOutStatus()));

  connect(motors[tblz], SIGNAL(changedMoving(bool)), SLOT(UpdateTblInOutStatus()));
  connect(motors[tblz], SIGNAL(changedUserPosition(double)), SLOT(UpdateTblInOutStatus()));

  connect(motors[gonio], SIGNAL(changedMoving(bool)), SLOT(updateEnergy()) );
  connect(motors[gonio], SIGNAL(changedUserPosition(double)), SLOT(updateEnergy()));
  //connect(motors[gonio], SIGNAL(changedUserPosition(double)), SLOT(updateDBragg()));
  //connect(motors[gonio], SIGNAL(changedMoving(bool)), SLOT(updateDBragg()));

  //connect(this, SIGNAL(expInBeamChanged(Expander::InOutPosition)), SLOT(UpdateInOutStatus()));
  //connect(this, SIGNAL(tblInBeamChanged(Expander::InOutPosition)), SLOT(UpdateInOutStatus()));
  updateConnection();

}

QHash<Expander::Motors,QCaMotor*> Expander::init_motors() {
  QHash<Expander::Motors,QCaMotor*> motret;
  motret[tilt]  = new QCaMotor(Expander::pvBaseName + "MOT3");
  motret[slide] = new QCaMotor(Expander::pvBaseName + "MOT4");
  motret[gonio] = new QCaMotor(Expander::pvBaseName + "MOT5");
  motret[inOut] = new QCaMotor(Expander::pvBaseName + "MOT6");
  motret[tbly] = new QCaMotor(Expander::pvTableBaseName + "Y");
  motret[tblz] = new QCaMotor(Expander::pvTableBaseName + "Z");
  return motret;
}

void Expander::wait_stop() {
  while ( isMoving() )
    qtWait(this, SIGNAL(motionChanged()));
}


void Expander::updateConnection() {
  bool con = true;
  /*foreach(QCaMotor * mot, motors)
    con &= mot->isConnected();*/
  con &= motors[tilt]->isConnected() &&
         motors[slide]->isConnected() &&
         motors[gonio]->isConnected() &&
         motors[inOut]->isConnected() &&
         motors[tbly]->isConnected() &&
         motors[tblz]->isConnected();
  //if(con) printf("con is true at some point\n");
  setConnected(con);
  if(isConnected()) {
    UpdateExpInOutStatus();
    UpdateTblInOutStatus();
    initEnergy();
  }
}

void Expander::updateMotion() {
  if (!isConnected())
    return;
  bool newMov = false;
  foreach(QCaMotor * mot, motors)
    newMov |= mot->isMoving();
  emit motionChanged(iAmMoving=newMov);
}

void Expander::setExpInBeam(bool val) {
  if ( ! isConnected() || isMoving() )
    return;
  Shutter1A sht1A;
  if (sht1A.mode() != Shutter1A::MONO ) {
    QMessageBox::warning(0,"Cannot move expander unless in mono", "To prevent you from moving the expander into white or mrt beams, this button will not work unless shutter mode is mono. Change mode and repeat or move it manually at your peril. (OUT is 18.)");
    return;
  }
  motors[inOut]->goUserPosition( val ? expanderInPosition : expanderOutPosition); // In=178 , out =18
}

void Expander::setTblInBeam(bool val) {
  if ( ! isConnected() || isMoving() )
    return;
  Shutter1A sht1A;
  if (sht1A.mode() != Shutter1A::MONO ) {
    QMessageBox::warning(0,"Cannot move TableZ unless in mono", "To prevent you from moving the IPASS table into white or mrt beams, this button will not work unless shutter mode is mono. Change mode and repeat or move it manually at your peril. (OUT is 0.)");
    return;
  }
  motors[tblz]->goUserPosition( val ? tableInPosition : tableOutPosition); // former In=178 , out =0
}

void Expander::stop() {
  foreach(QCaMotor * mot, motors)
    mot->stop();
}

/*void Expander::UpdateInOutStatus() {
  if ( ! motors[inOut]->isConnected() )
    return;

  if (_tblInBeam==OUTBEAM && _expInBeam==OUTBEAM ){
    _inBeam=OUTBEAM;
  //  warn("_totalInBeam is OUTBEAM", objectName());
  }
  else{
   _inBeam=INBEAM;
   //warn("_totalInBeam is INBEAM", objectName());
  }
  emit inBeamChanged(_inBeam);
}*/

void Expander::UpdateTblInOutStatus() {
  if ( ! motors[tblz]->isConnected() )
    return;

  double newInOut = motors[tblz]->getUserPosition();
  if (motors[inOut]->isMoving())
    _tblInBeam = MOVING;
  else if ( qAbs(newInOut - tableInPosition) <= 0.1) // 1 micron - unsertanty in Z
    _tblInBeam = INBEAM;
  else if ( qAbs(newInOut)- tableOutPosition <= 0.1){
    _tblInBeam = OUTBEAM;
  //  warn("_tblInBeam is OUTBEAM", objectName());
  }
  else {
    _tblInBeam = BETWEEN;
  }
  emit tblInBeamChanged(_tblInBeam);
}

void Expander::UpdateExpInOutStatus(){
  if ( ! motors[inOut]->isConnected() )
    return;

  double newInOut = motors[inOut]->getUserPosition();
  //warn("newInOut is "+ QString::number(newInOut), objectName());
  if (motors[inOut]->isMoving()){
    _expInBeam = MOVING;
  //  warn("_expInBeam is MOVING", objectName());
  }
  else if ( qAbs(newInOut - expanderInPosition) <= 0.1 ){ // 1 micron - unsertanty in Z
    _expInBeam = INBEAM;
  //  warn("_expInBeam is INBEAM", objectName());
  }
  else if ( qAbs(newInOut - expanderOutPosition ) <= 0.1 ){
    _expInBeam = OUTBEAM;
  //  warn("_expInBeam is OUTBEAM", objectName());
  }
  else {
    _expInBeam = BETWEEN;
  }

  emit expInBeamChanged(_expInBeam);
}

void Expander::updateEnergy() {
  if ( ! motors[gonio]->isConnected() )
    return;
  const double mAngle = motors[gonio]->getUserPosition();
  double ddBragg = 0;
  if (_useDBragg) ddBragg = dBragg();
  _energy = (mAngle-ddBragg-Expander::theIntercept)/Expander::theGradient;
  emit energyChanged(_energy);
}

void Expander::setEnergy(double enrg, bool keepDBragg) {

  if ( ! isConnected() || isMoving() )
    return;

  if ( enrg < energyRange.first || enrg > energyRange.second ) {
    QMessageBox::warning(0,"Cannot move expander gonio to that energy", "Requested energy (" + QString::number(enrg) + "keV) is out of the allowed range (" + QString::number(energyRange.first) + "," + QString::number(energyRange.second) + ")kEv. Ignoring the request.");
    return;
  }

  const double ddbragg = keepDBragg ? dBragg() : 0;
  std::cout << "ddbragg is: " << ddbragg <<std::endl; 
  motors[gonio]->goUserPosition( Expander::theGradient*enrg+Expander::theIntercept + ddbragg , QCaMotor::STARTED);
  QTimer::singleShot(0, this, SLOT(updateEnergy()));
  QTimer::singleShot(0, this, SLOT(updateDBragg()));
}

void Expander::updateDBragg() {
  if ( ! motors[gonio]->isConnected() || motors[gonio]->isMoving() )
      return;
  double aBragg = motors[gonio]->getUserPosition() - (Expander::theGradient*energy()+Expander::theIntercept);
  emit dBraggChanged(aBragg);
}

void Expander::setDBragg(double val) {
 if ( ! isConnected() || isMoving() )
    return;
 _dBragg = val;
 motors[gonio]->goUserPosition(val+Expander::theGradient*energy()+Expander::theIntercept, QCaMotor::STARTED);
}

void Expander::initEnergy() {

  if ( ! motors[gonio]->isConnected() )
    return;
  const double mAngle = motors[gonio]->getUserPosition();
  _energy = (mAngle-Expander::theIntercept)/Expander::theGradient;
  updateDBragg();
  emit energyChanged(_energy);
}

void Expander::setUseDBragg(int val){
  if (val == 2) _useDBragg = true;
  else _useDBragg = false;
}
