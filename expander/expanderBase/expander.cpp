#include "expander.h"
#include "error.h"
#include <qtpv.h>
#include "shutterFE.h"

const QString Expander::pvBaseName = "SR08ID01EXP01:";
const QString Expander::pvTableBaseName = "SR08ID01TBL13:";
const QHash<Expander::Motors,QCaMotor*> Expander::motors=Expander::init_motors();

Expander::Expander(QObject *parent) :
  Component("Expander", parent),
  iAmMoving(false),
  _inBeam(BETWEEN),
  _tblInBeam(BETWEEN)
{
  foreach(QCaMotor * mot, motors) {
    connect(mot, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
    connect(mot, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
  }
  connect(motors[inOut], SIGNAL(changedMoving(bool)), SLOT(UpdateInOutStatus()));
  connect(motors[inOut], SIGNAL(changedUserPosition(double)), SLOT(UpdateInOutStatus()));

  connect(motors[tblz], SIGNAL(changedMoving(bool)), SLOT(UpdateTblInOutStatus()));
  connect(motors[tblz], SIGNAL(changedUserPosition(double)), SLOT(UpdateTblInOutStatus()));

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
  if(isConnected()) UpdateInOutStatus();
}

void Expander::updateMotion() {
  if (!isConnected())
    return;
  bool newMov = false;
  foreach(QCaMotor * mot, motors)
    newMov |= mot->isMoving();
  emit motionChanged(iAmMoving=newMov);
}

void Expander::setInBeam(bool val) {
  if ( ! isConnected() || isMoving() )
    return;
  if ( ! ShutterFE::setOpenedS(false,true) ) {
    warn("Can't close the FE shutter."
         " Switching Expander mode failed."
         " Try to repeat or do it manually.", this);
    return;
  }
  motors[inOut]->goUserPosition( val ? 178 : 18); // In=178 , out =18
}

void Expander::setTblInBeam(bool val) {
  if ( ! isConnected() || isMoving() )
    return;
  if ( ! ShutterFE::setOpenedS(false,true) ) {
    warn("Can't close the FE shutter."
         " Switching Expander mode failed."
         " Try to repeat or do it manually.", this);
    return;
  }
  motors[tblz]->goUserPosition( val ? 178 : 0); // In=178 , out =0
}

void Expander::stop() {
  foreach(QCaMotor * mot, motors)
    mot->stop();
}

void Expander::UpdateInOutStatus() {
  if ( ! motors[inOut]->isConnected() )
    return;

  double newInOut = motors[inOut]->getUserPosition();
  if (motors[inOut]->isMoving())
    _inBeam = MOVING;
  else if ( qAbs(newInOut) <= 178.1 && qAbs(newInOut) >= 177.9 ) // 1 micron - unsertanty in Z
    _inBeam = INBEAM;
  else if ( qAbs(newInOut) <= 18.1 && qAbs(newInOut) >= 17.9)
    _inBeam = OUTBEAM;
  else {
    _inBeam = BETWEEN;
    warn("Z position of the expander (" + QString::number(newInOut) +
         ") is between \"in\" and \"out\" destinations.",
         objectName());
  }
  emit inBeamChanged(_inBeam);
}

void Expander::UpdateTblInOutStatus() {
  if ( ! motors[tblz]->isConnected() )
    return;

  double newInOut = motors[tblz]->getUserPosition();
  if (motors[inOut]->isMoving())
    _tblInBeam = MOVING;
  else if ( qAbs(newInOut) <= 178.1 && qAbs(newInOut) >= 177.9 ) // 1 micron - unsertanty in Z
    _tblInBeam = INBEAM;
  else if ( qAbs(newInOut) <= 0.1)
    _tblInBeam = OUTBEAM;
  else {
    _tblInBeam = BETWEEN;
    warn("Z position of the BCT Table Z (" + QString::number(newInOut) +
         ") is between \"in\" and \"out\" destinations.",
         objectName());
  }
  emit tblInBeamChanged(_tblInBeam);
}