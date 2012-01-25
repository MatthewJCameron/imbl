#include "mono.h"
#include "error.h"


static const double esinbq = 1.1414666995476608627; // E * sin(Bragg) / sqrt (h*h + k*k + l*l)
static const double sqrt111 = sqrt(3);
static const double sqrt311 = sqrt(11);

double energy2bragg(double energy, Mono::Diffraction diff) {
  if (energy <= 0.0)
    return 0;
  return asin(esinbq * (diff==Mono::Si111 ? sqrt111 : sqrt311) / energy )
      * 180.0 / M_PI ;
}


const QHash<Mono::Motors,QCaMotor*> Mono::motors=Mono::init_motors();
const QPair<double,double> Mono::energyRange = qMakePair<double,double>(16.0,195.0);


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

double Mono::motorAngle(double enrg, int crystal, Diffraction diff) {
  switch (crystal) {
  case 1:
    return energy2bragg(enrg, diff) + (diff == Si111 ? -1.0*alpha : alpha);
  case 2:
    return energy2bragg(enrg, diff) + (diff == Si111 ? alpha : -1.0*alpha);
  default:
    return 0;
  }
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

  const double mAngle = motors[Bragg1]->getUserPosition();
  double bAngle;
  if ( mAngle <= alpha ) {
    _diff = Si111;
    bAngle = mAngle + alpha;
  } else {
    _diff = Si311;
    bAngle = mAngle - alpha;
  }

  if ( ( bAngle <= 0 || bAngle >= 90 ) ) {
    if ( ! motors[Bragg1]->isMoving() )
      warn("Meaningless bragg angle for the first crystal:" + QString::number(bAngle) + ".",
           objectName());
    return;
  }

  const double newEnergy = esinbq * ( diffraction()==Si111 ? sqrt111 : sqrt311 )
      / sin(bAngle*M_PI/180);
  if (newEnergy != _energy) {
    _energy = newEnergy;
    updateBragg2();
    updateX();
    emit energyChanged(_energy);
  }

}


void Mono::updateBragg2() {
  if ( ! isConnected() || motors[Bragg1]->isMoving() )
    return;
  const double delta = motors[Bragg2]->getUserPosition()
      - motorAngle(energy(), 2,diffraction());
  const double newDBragg = 1.0e6*delta*M_PI/180.0; //murad
  if (newDBragg != _dBragg)
    emit dBraggChanged(_dBragg=newDBragg);
}


void Mono::updateX() {
  if ( ! isConnected() )
    return;
  const double xDist = (zDist+dZ()) / tan(2*energy2bragg(energy(),diffraction())*M_PI/180);
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
  setEnergy(val, diffraction(), keepDBragg, keepDX);
}


void Mono::setEnergy(double enrg, Mono::Diffraction diff, bool keepDBragg, bool keepDX) {

  if ( ! isConnected() || isMoving() )
    return;

  if ( enrg < energyRange.first || enrg > energyRange.second ) {
    warn("Requested energy (" + QString::number(enrg) + "kEv) is out of the allowed range"
         " (" + QString::number(energyRange.first) + "," + QString::number(energyRange.second) +
         ")kEv. Ignoring the request.",
         objectName() );
    return;
  }
  if ( diff == Si111 && enrg > maxEnergy111 ) {
    warn("Requested energy (" + QString::number(enrg) + "kEv) is greater than the"
         "  maximum for the requested diffraction type Si[111]:"
         " " + QString::number(maxEnergy111) + "kEv. Ignoring the request.",
         objectName() );
    return;
  }
  if ( diff == Si311 && enrg < minEnergy311 ) {
    warn("Requested energy (" + QString::number(enrg) + "kEv) is smaller than the"
         "  minimum for the requested diffraction type Si[311]:"
         " " + QString::number(minEnergy311) + "kEv. Ignoring the request.",
         objectName() );
    return;
  }

  double braggA = energy2bragg(enrg, diff);
  if ( braggA <= 0.0 || braggA >= 90.0 ) { // shoud never happen because of the ifs above.
    warn("Impossible Bragg angle " + QString::number(braggA) + "."
         " Likely to be developer's bug.");
    return;
  }

  motors[Bragg1]->goUserPosition( motorAngle(enrg, 1, diff),
                                 QCaMotor::STARTED);
  motors[Bragg2]->goUserPosition( motorAngle(enrg, 2, diff)
                                  + ( keepDBragg ? dBragg() : 0 ),
                                  QCaMotor::STARTED);
  motors[Xdist]->goUserPosition( (zDist+dZ()) / tan(2*braggA*M_PI/180) +
                                 + ( keepDX ? dX() : 0 ),
                                 QCaMotor::STARTED);

}


void Mono::setDBragg(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Bragg2]->goUserPosition( motorAngle(energy(), 2, diffraction())
                                  + val * 180 / (1.0e06 * M_PI),
                                  QCaMotor::STARTED);
}


void Mono::setDX(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Xdist]->goUserPosition
      ( (zDist+dZ()) / tan(2*energy2bragg(energy(),diffraction())*M_PI/180) + val,
        QCaMotor::STARTED);
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
















