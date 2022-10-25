#include "expander.h"
#include "error.h"
#include <qtpv.h>
#include "shutterFE.h"
#include "shutter1A.h"
#include <QMessageBox>

const QString Expander::pvBaseName = "SR08ID01EXP01:";
const QString Expander::pvTableBaseName = "SR08ID01TBL13:";
const QHash<Expander::Motors,QCaMotor*> Expander::motors=Expander::init_motors();

Expander::Expander(QObject *parent) :
  Component("Expander", parent),
  iAmMoving(false),
  //_inBeam(BETWEEN),
  _tblInBeam(BETWEEN),
  _expInBeam(BETWEEN)
{
  foreach(QCaMotor * mot, motors) {
    connect(mot, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
    connect(mot, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
  }
  connect(motors[inOut], SIGNAL(changedMoving(bool)), SLOT(UpdateExpInOutStatus()));
  connect(motors[inOut], SIGNAL(changedUserPosition(double)), SLOT(UpdateExpInOutStatus()));

  connect(motors[tblz], SIGNAL(changedMoving(bool)), SLOT(UpdateTblInOutStatus()));
  connect(motors[tblz], SIGNAL(changedUserPosition(double)), SLOT(UpdateTblInOutStatus()));

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
  motors[inOut]->goUserPosition( val ? 176 : 18); // In=178 , out =18
}

void Expander::setTblInBeam(bool val) {
  if ( ! isConnected() || isMoving() )
    return;
  Shutter1A sht1A;
  if (sht1A.mode() != Shutter1A::MONO ) {
    QMessageBox::warning(0,"Cannot move TableZ unless in mono", "To prevent you from moving the IPASS table into white or mrt beams, this button will not work unless shutter mode is mono. Change mode and repeat or move it manually at your peril. (OUT is 0.)");
    return;
  }
  motors[tblz]->goUserPosition( val ? 178.723 : 0); // In=178 , out =0
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
  else if ( qAbs(newInOut -178.723) <= 0.1) // 1 micron - unsertanty in Z
    _tblInBeam = INBEAM;
  else if ( qAbs(newInOut) <= 0.1){
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
  else if ( qAbs(newInOut - 176.0) <= 0.1 ){ // 1 micron - unsertanty in Z
    _expInBeam = INBEAM;
  //  warn("_expInBeam is INBEAM", objectName());
  }
  else if ( qAbs(newInOut - 18.0 ) <= 0.1 ){
    _expInBeam = OUTBEAM;
  //  warn("_expInBeam is OUTBEAM", objectName());
  }
  else {
    _expInBeam = BETWEEN;
  }

  emit expInBeamChanged(_expInBeam);
}