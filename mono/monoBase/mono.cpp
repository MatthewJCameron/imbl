#include "mono.h"
#include "error.h"

const QHash<Mono::Motors,QCaMotor*> Mono::motors=Mono::init_motors();
const QPair<double,double> Mono::energyRange = qMakePair<double,double>(17.0,200.0);


Mono::Mono(QObject *parent) :
  Component("Monochromator", parent),
  iAmMoving(false),
  _energy(0),
  _dBragg(0),
  _dX(0),
  _dZ(0),
  _bend1(0),
  _bend2(0),
  _inBeam(BETWEEN)
{

  foreach(QCaMotor * mot, motors) {
    connect(mot, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
    connect(mot, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
  }
  connect(motors[Z1],     SIGNAL(changedMoving(bool)), SLOT(updateZ1()));

  connect(motors[Bragg1], SIGNAL(changedUserPosition(double)), SLOT(updateBragg1()));
  connect(motors[Bragg2], SIGNAL(changedUserPosition(double)), SLOT(updateBragg2()));
  connect(motors[Tilt1],  SIGNAL(changedUserPosition(double)), SLOT(updateTilt1()));
  connect(motors[Tilt2],  SIGNAL(changedUserPosition(double)), SLOT(updateTilt2()));
  connect(motors[Z1],     SIGNAL(changedUserPosition(double)), SLOT(updateZ1()));
  connect(motors[Z2],     SIGNAL(changedUserPosition(double)), SLOT(updateZ2()));
  connect(motors[Xdist],  SIGNAL(changedUserPosition(double)), SLOT(updateX()));
  connect(motors[Bend1],  SIGNAL(changedUserPosition(double)), SLOT(updateBend1()));
  connect(motors[Bend2],  SIGNAL(changedUserPosition(double)), SLOT(updateBend2()));

  updateConnection();

}

QHash<Mono::Motors,QCaMotor*> Mono::init_motors() {
  QHash<Mono::Motors,QCaMotor*> motret;
  motret[Bragg1] = new QCaMotor("");
  motret[Bragg2] = new QCaMotor("");
  motret[Tilt1]  = new QCaMotor("");
  motret[Tilt2]  = new QCaMotor("");
  motret[Z1]     = new QCaMotor("");
  motret[Z2]     = new QCaMotor("");
  motret[Xdist]  = new QCaMotor("");
  motret[Bend1]  = new QCaMotor("");
  motret[Bend2]  = new QCaMotor("");
  return motret;
}

void Mono::wait_stop() {
  while ( isMoving() )
    qtWait(this, SIGNAL(motionChanged()));
}


void Mono::updateConnection() {
  bool con = true;
  foreach(QCaMotor * mot, motors)
    con &= mot->isConnected();
  setConnected(con);
  if ( isConnected() ) {
    updateBragg1();
    updateBragg2();
    updateTilt1();
    updateTilt2();
    updateZ1();
    updateZ2();
    updateX();
    updateBend1();
    updateBend2();
  }
}


void Mono::updateMotion() {
  bool newMov = false;
  foreach(QCaMotor * mot, motors)
    newMov |= mot->isMoving();
  if (newMov != iAmMoving)
    emit motionChanged(iAmMoving=newMov);
}


void Mono::updateBragg1() {
  if ( ! isConnected() )
    return;
  if ( bragg() <= 0 || bragg() >= 90) {
    warn("Meaningless bragg angle for the first crystal:" + QString::number(bragg()) + ".",
         objectName());
    return;
  }
  const double newEnergy = esinb / sin( M_PI * bragg() / 180);
  if (newEnergy != _energy) {
    _energy = newEnergy;
    updateBragg2();
    updateX();
    emit energyChanged(_energy);
  }
}


void Mono::updateBragg2() {
  if ( ! isConnected() )
    return;
  double bragg2 = motors[Bragg2]->getUserPosition();
  if (bragg2 <= 0 || bragg2 >= 90)
    warn("Meaningless bragg angle for the second crystal:" + QString::number(bragg2) + ".",
         objectName());
  double newDBragg = 1.0e6*(bragg2-bragg())*M_PI/180.0; //murad
  if (newDBragg != _dBragg)
    emit dBraggChanged(_dBragg=newDBragg);
}


void Mono::updateX() {
  if ( ! isConnected() )
    return;
  const double xDist = (zDist+dZ()) * tan(bragg());
  const double newDX = motors[Xdist]->getUserPosition() - xDist;
  if (newDX != _dX)
    emit dXChanged(_dX=newDX);
}


void Mono::updateZ1() {

  if ( ! isConnected() )
    return;

  double newZ1 = motors[Z1]->getUserPosition();
  InOutPosition newpos;
  if (motors[Z1]->isMoving())
    newpos = MOVING;
  else if ( newZ1 == 0.0 )
    newpos = INBEAM;
  else if ( newZ1 >= zOut )
    newpos = OUTBEAM;
  else {
    newpos = BETWEEN;
    warn("Z position of the first crystall (" + QString::number(newZ1) +
         ") is between \"in\" and \"out\" destinations.",
         objectName());
  }

  if (newpos != _inBeam)
    emit inBeamChanged(_inBeam=newpos);

}


void Mono::updateZ2() {
  if ( ! isConnected() )
    return;
  double newZ2 = motors[Z2]->getUserPosition();
  if ( newZ2-zDist != _dZ ) {
    _dZ = newZ2-zDist;
    updateX();
    emit dZChanged(_dZ);
  }
}


void Mono::updateTilt1() {
  if ( ! isConnected() )
    return;
  double newT1 = motors[Tilt1]->getUserPosition();
  if (newT1 != _tilt1)
    emit tilt1Changed(_tilt1=newT1);
}


void Mono::updateTilt2() {
  if ( ! isConnected() )
    return;
  double newT2 = motors[Tilt2]->getUserPosition();
  if (newT2 != _tilt2)
    emit tilt2Changed(_tilt2=newT2);
}


void Mono::updateBend1() {
  if ( ! isConnected() )
    return;
  double newB1 = motors[Bend1]->getUserPosition();
  if (newB1 != _bend1)
    emit bend1Changed(_bend1=newB1);
}


void Mono::updateBend2() {
  if ( ! isConnected() )
    return;
  double newB2 = motors[Bend2]->getUserPosition();
  if (newB2 != _bend2)
    emit bend2Changed(_bend2=newB2);
}


void Mono::setEnergy(double val, bool keepDBragg, bool keepDX) {
  if ( ! isConnected() || isMoving() )
    return;
  if ( val < energyRange.first || val > energyRange.second ) {
    warn("Requested energy (" + QString::number(val) + "kEv) is out of the allowed range"
         " (" + QString::number(energyRange.first) + "," + QString::number(energyRange.second) +
         ")kEv. Ignoring the request.",
         objectName() );
    return;
  }
  double braggD = 180 * asin(esinb/val) / M_PI ;
  motors[Bragg1]->goUserPosition(braggD, QCaMotor::STARTED);
  motors[Bragg2]->goUserPosition(braggD + ( keepDBragg ? dBragg() : 0 ),
                          QCaMotor::STARTED);
  motors[Xdist]->goUserPosition( (zDist+dZ()) * esinb / sqrt(val*val-esinb*esinb)
                                 + ( keepDX ? dX() : 0 ), QCaMotor::STARTED);
}


void Mono::setDBragg(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Bragg2]->goUserPosition(bragg() + val, QCaMotor::STARTED);
}


void Mono::setDX(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Xdist]->goUserPosition( (zDist+dZ()) * tan(bragg()) + val );
}


void Mono::setInBeam(bool val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Z1]->goUserPosition( val ? 0 : zOut);
}


void Mono::setDZ(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Z2]->goUserPosition(zDist+val);
}


void Mono::setTilt1(double val) {
  if ( ! isConnected() )
    return;
  motors[Tilt1]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::setTilt2(double val) {
  if ( ! isConnected() )
    return;
  motors[Tilt2]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::setBend1(double val) {
  if ( ! isConnected() )
    return;
  motors[Bend1]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::setBend2(double val) {
  if ( ! isConnected() )
    return;
  motors[Bend2]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::stop() {
  foreach(QCaMotor * mot, motors)
    mot->stop();
}
















