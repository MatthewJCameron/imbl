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
  _zSeparation(0),
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
  connect(motors[Bend1f],  SIGNAL(changedUserPosition(double)), SLOT(updateBend1f()));
  connect(motors[Bend2f],  SIGNAL(changedUserPosition(double)), SLOT(updateBend2f()));
  connect(motors[Bend1b],  SIGNAL(changedUserPosition(double)), SLOT(updateBend1b()));
  connect(motors[Bend2b],  SIGNAL(changedUserPosition(double)), SLOT(updateBend2b()));


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
  motret[Bend1f]  = new QCaMotor("");
  motret[Bend2f]  = new QCaMotor("");
  motret[Bend1b]  = new QCaMotor("");
  motret[Bend2b]  = new QCaMotor("");
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
    updateBend1f();
    updateBend2f();
    updateBend1b();
    updateBend2b();
  }
}


void Mono::updateMotion() {
  bool newMov = false;
  foreach(QCaMotor * mot, motors)
    newMov |= mot->isMoving();
  emit motionChanged(iAmMoving=newMov);
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


void Mono::updateBragg1() {
  if ( ! isConnected() || motors[Bragg2]->isMoving() )
    return;
  const double delta = motors[Bragg1]->getUserPosition()
      - motorAngle(energy(), 2, diffraction());
  _dBragg = 1.0e6*delta*M_PI/180.0; //murad
  emit dBraggChanged(_dBragg);
}


void Mono::updateBragg2() {

  if ( ! isConnected() )
    return;

  const double mAngle = motors[Bragg2]->getUserPosition();
  double bAngle;
  if ( mAngle >= alpha ) {
    _diff = Si111;
    bAngle = mAngle - alpha;
  } else {
    _diff = Si311;
    bAngle = mAngle + alpha;
  }

  if ( ( bAngle <= 0 || bAngle >= 90 ) ) {
    if ( ! motors[Bragg2]->isMoving() )
      warn("Meaningless bragg angle for the second crystal:" + QString::number(bAngle) + ".",
           objectName());
    return;
  }

  _energy = esinbq * ( diffraction()==Si111 ? sqrt111 : sqrt311 ) / sin(bAngle*M_PI/180);
  updateBragg1();
  updateX();
  emit energyChanged(_energy);

}


void Mono::updateX() {
  if ( ! isConnected() )
    return;
  const double xDist = zSeparation() / tan(2*energy2bragg(energy(),diffraction())*M_PI/180);
  _dX = motors[Xdist]->getUserPosition() - xDist;
  emit dXChanged(_dX);
}


void Mono::updateZ1() {

  if ( ! isConnected() )
    return;

  double newZ1 = motors[Z1]->getUserPosition();
  if (motors[Z1]->isMoving())
    _inBeam = MOVING;
  else if ( newZ1 == 0.0 )
    _inBeam = INBEAM;
  else if ( newZ1 >= zOut )
    _inBeam = OUTBEAM;
  else {
    _inBeam = BETWEEN;
    warn("Z position of the first crystall (" + QString::number(newZ1) +
         ") is between \"in\" and \"out\" destinations.",
         objectName());
  }
  emit inBeamChanged(_inBeam);

}


void Mono::updateZ2() {
  if ( ! isConnected() )
    return;
  _zSeparation = motors[Z2]->getUserPosition();
  updateX();
  emit zSeparationChanged(_zSeparation);
}


void Mono::updateTilt1() {
  if ( ! isConnected() )
    return;
  emit tilt1Changed(tilt1());
}


void Mono::updateTilt2() {
  if ( ! isConnected() )
    return;
  emit tilt1Changed(tilt2());
}


void Mono::updateBend1f() {
  if ( ! isConnected() )
    return;
  emit bend1frontChanged(bend1front());
}


void Mono::updateBend2f() {
  if ( ! isConnected() )
    return;
  emit bend2frontChanged(bend2front());
}


void Mono::updateBend1b() {
  if ( ! isConnected() )
    return;
  emit bend1backChanged(bend1back());
}


void Mono::updateBend2b() {
  if ( ! isConnected() )
    return;
  emit bend2backChanged(bend2back());
}






void Mono::setEnergy(double val, bool keepDBragg, bool keepDX) {
  setEnergy(val, diffraction(), keepDBragg, keepDX);
}


void Mono::setEnergy(double enrg, Mono::Diffraction diff, bool keepDBragg, bool keepDX) {

  QTimer::singleShot(0, this, SLOT(updateBragg1()));
  QTimer::singleShot(0, this, SLOT(updateBragg2()));
  QTimer::singleShot(0, this, SLOT(updateX()));

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

  motors[Bragg1]->goUserPosition( motorAngle(enrg, 1, diff)
                                  + ( keepDBragg ? dBragg() : 0 ),
                                  QCaMotor::STARTED);
  motors[Bragg2]->goUserPosition( motorAngle(enrg, 2, diff),
                                  QCaMotor::STARTED);
  motors[Xdist]->goUserPosition( zSeparation() / tan(2*braggA*M_PI/180) +
                                 + ( keepDX ? dX() : 0 ),
                                 QCaMotor::STARTED);

}


void Mono::setDBragg(double val) {
  QTimer::singleShot(0, this, SLOT(updateBragg1()));
  if ( ! isConnected() || isMoving() )
    return;
  motors[Bragg1]->goUserPosition( motorAngle(energy(), 1, diffraction())
                                  + val * 180 / (1.0e06 * M_PI),
                                  QCaMotor::STARTED);
}


void Mono::setDX(double val) {
  QTimer::singleShot(0, this, SLOT(updateX()));
  if ( ! isConnected() || isMoving() )
    return;
  motors[Xdist]->goUserPosition
      ( zSeparation() / tan(2*energy2bragg(energy(),diffraction())*M_PI/180) + val,
        QCaMotor::STARTED);
}


void Mono::setInBeam(bool val) {
  QTimer::singleShot(0, this, SLOT(updateZ1()));
  if ( ! isConnected() || isMoving() )
    return;
  motors[Z1]->goUserPosition( val ? 0 : zOut);
}


void Mono::setZseparation(double val) {
  QTimer::singleShot(0, this, SLOT(updateZ2()));
  if ( ! isConnected() || isMoving() )
    return;
  motors[Z2]->goUserPosition(val, QCaMotor::STOPPED);
  setEnergy(energy(), diffraction());
}


void Mono::setTilt1(double val) {
  QTimer::singleShot(0, this, SLOT(updateTilt1()));
  if ( ! isConnected() )
    return;
  motors[Tilt1]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::setTilt2(double val) {
  QTimer::singleShot(0, this, SLOT(updateTilt2()));
  if ( ! isConnected() )
    return;
  motors[Tilt2]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::setBend1front(double val) {
  QTimer::singleShot(0, this, SLOT(updateBend1f()));
  if ( ! isConnected() )
    return;
  motors[Bend1f]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::setBend2front(double val) {
  QTimer::singleShot(0, this, SLOT(updateBend2f()));
  if ( ! isConnected() )
    return;
  motors[Bend2f]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::setBend1back(double val) {
  QTimer::singleShot(0, this, SLOT(updateBend1b()));
  if ( ! isConnected() )
    return;
  motors[Bend1b]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::setBend2back(double val) {
  QTimer::singleShot(0, this, SLOT(updateBend2b()));
  if ( ! isConnected() )
    return;
  motors[Bend2b]->goUserPosition(val, QCaMotor::STARTED);
}


void Mono::stop() {
  foreach(QCaMotor * mot, motors)
    mot->stop();
}
















