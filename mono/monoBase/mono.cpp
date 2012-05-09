#include "mono.h"
#include "error.h"
#include <qtpv.h>


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
const QHash < Mono::Motors, QPair<double,double> > Mono::travelRanges = Mono::init_ranges();
QEpicsPv * Mono::Bragg1Enc = new QEpicsPv("SR08ID01MCS01:BRAGG1:ENCODER");
QEpicsPv * Mono::Bragg2Enc = new QEpicsPv("SR08ID01MCS01:BRAGG2:ENCODER");
QEpicsPv * Mono::XdistEnc = new QEpicsPv("SR08ID01MCS01:X:ENCODER");
QEpicsPv * Mono::Bragg1EncLoss = new QEpicsPv("SR08ID01MCS01:BRAGG1:ELOSS");
QEpicsPv * Mono::Bragg2EncLoss = new QEpicsPv("SR08ID01MCS01:BRAGG2:ELOSS");
QEpicsPv * Mono::XdistEncLoss = new QEpicsPv("SR08ID01MCS01:X:ELOSS");

const QPair<double,double> Mono::energyRange = qMakePair<double,double>(16.0,195.0);



Mono::Mono(QObject *parent) :
  Component("Monochromator", parent),
  iAmMoving(false),
  _energy(0),
  _dBragg(0),
  _dX(0),
  _zSeparation(standardZseparation),
  _dZ(0),
  b1ob(0),
  b1ib(0),
  b2ob(0),
  b2ib(0),
  _inBeam(BETWEEN)
{

  foreach (Motors motk, motors.keys()) {
    calibratedMotors[motk]  = true;
    incalibration[motk] = false;
  }

  foreach(QCaMotor * mot, motors) {
    connect(mot, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
    connect(mot, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
    connect(mot, SIGNAL(changedUserPosition(double)), SLOT(updateCalibration()));
    connect(mot, SIGNAL(changedLoLimitStatus(bool)), SLOT(updateCalibration()));
    connect(mot, SIGNAL(changedHiLimitStatus(bool)), SLOT(updateCalibration()));
  }
  connect(motors[Z1],     SIGNAL(changedMoving(bool)), SLOT(updateZ1()));
  connect(motors[Bragg1], SIGNAL(changedUserPosition(double)), SLOT(updateBragg1()));
  connect(motors[Bragg2], SIGNAL(changedUserPosition(double)), SLOT(updateBragg2()));
  connect(motors[Tilt1],  SIGNAL(changedUserPosition(double)), SLOT(updateTilt1()));
  connect(motors[Tilt2],  SIGNAL(changedUserPosition(double)), SLOT(updateTilt2()));
  connect(motors[Z1],     SIGNAL(changedUserPosition(double)), SLOT(updateZ1()));
  connect(motors[Z2],     SIGNAL(changedUserPosition(double)), SLOT(updateZ2()));
  connect(motors[Xdist],  SIGNAL(changedUserPosition(double)), SLOT(updateX()));
  connect(motors[Bend1ob],  SIGNAL(changedUserPosition(double)), SLOT(updateBend1ob()));
  connect(motors[Bend2ob],  SIGNAL(changedUserPosition(double)), SLOT(updateBend2ob()));
  connect(motors[Bend1ib],  SIGNAL(changedUserPosition(double)), SLOT(updateBend1ib()));
  connect(motors[Bend2ib],  SIGNAL(changedUserPosition(double)), SLOT(updateBend2ib()));

  connect(Bragg1Enc, SIGNAL(valueUpdated(QVariant)), SLOT(updateCalibration()));
  connect(Bragg2Enc, SIGNAL(valueUpdated(QVariant)), SLOT(updateCalibration()));
  connect(XdistEnc, SIGNAL(valueUpdated(QVariant)), SLOT(updateCalibration()));
  connect(Bragg1EncLoss, SIGNAL(valueUpdated(QVariant)), SLOT(updateCalibration()));
  connect(Bragg2EncLoss, SIGNAL(valueUpdated(QVariant)), SLOT(updateCalibration()));
  connect(XdistEncLoss, SIGNAL(valueUpdated(QVariant)), SLOT(updateCalibration()));




  updateConnection();

}

QHash<Mono::Motors,QCaMotor*> Mono::init_motors() {
  QHash<Mono::Motors,QCaMotor*> motret;
  motret[Xdist]  = new QCaMotor("SR08ID01MCS01:X");
  motret[Bragg1] = new QCaMotor("SR08ID01MCS01:BRAGG1");
  motret[Bragg2] = new QCaMotor("SR08ID01MCS01:BRAGG2");
  motret[Tilt1]  = new QCaMotor("SR08ID01MCS02:TILT1");
  motret[Tilt2]  = new QCaMotor("SR08ID01MCS02:TILT2");
  motret[Z1]     = new QCaMotor("SR08ID01MCS01:Z1");
  motret[Z2]     = new QCaMotor("SR08ID01MCS01:Z2");
  motret[Bend1ob]  = new QCaMotor("SR08ID01MCS02:BENDER1IB");
  motret[Bend2ob]  = new QCaMotor("SR08ID01MCS02:BENDER2IB");
  motret[Bend1ib]  = new QCaMotor("SR08ID01MCS02:BENDER1OB");
  motret[Bend2ib]  = new QCaMotor("SR08ID01MCS02:BENDER2OB");
  return motret;
}


QHash<Mono::Motors, QPair<double,double> > Mono::init_ranges() {
  QHash< Mono::Motors,QPair<double,double> > rangret;
  rangret[Xdist]  = qMakePair(84.04,632.51);
  rangret[Bragg1] = qMakePair(-16.27,23.49);
  rangret[Bragg2] = qMakePair(-16.04,24.13);
  rangret[Tilt1]  = qMakePair(-1.51,2.46);
  rangret[Tilt2]  = qMakePair(-5.10,4.81);
  rangret[Z1]     = qMakePair(-49.63,4.16);
  rangret[Z2]     = qMakePair(16.07,34.43);
  rangret[Bend1ob]  = qMakePair(-0.004,15.195);
  rangret[Bend2ob]  = qMakePair(-0.093,14.95);
  rangret[Bend1ib]  = qMakePair(-0.003,15.296);
  rangret[Bend2ib]  = qMakePair(-0.009,14.959);
  return rangret;
}




void Mono::wait_stop() {
  while ( isMoving() )
    qtWait(this, SIGNAL(motionChanged()));
}


void Mono::updateConnection() {
  bool con = true;
  foreach(QCaMotor * mot, motors)
    con &= mot->isConnected();
  con &=
      Bragg1Enc->isConnected() && Bragg1EncLoss->isConnected() &&
      Bragg2Enc->isConnected() && Bragg2EncLoss->isConnected() &&
      XdistEnc->isConnected() && XdistEncLoss->isConnected();

  setConnected(con);

  if ( isConnected() ) {
    updateBragg1();
    updateBragg2();
    updateTilt1();
    updateTilt2();
    updateZ1();
    updateZ2();
    updateX();
    updateBend1ob();
    updateBend2ob();
    updateBend1ib();
    updateBend2ib();
    emit zSeparationChanged(zSeparation());
  }
}


void Mono::updateMotion() {
  if (!isConnected())
    return;
  bool newMov = false;
  foreach(QCaMotor * mot, motors)
    newMov |= mot->isMoving();
  emit motionChanged(iAmMoving=newMov);
}


bool checkCalibrate (QCaMotor * mot, QPair<double,double> range, double allowance) {
  return
      mot->getUserPosition() >= range.first - 0.1 &&
      mot->getUserPosition() <= range.second + 0.1 &&
      ( ! mot->getLoLimitStatus() || mot->getUserPosition() <= range.first + allowance ) &&
      ( ! mot->getHiLimitStatus() || mot->getUserPosition() >= range.second - allowance );
}


void Mono::updateCalibration() {

  if ( ! isConnected() ||
       ( static_cast<QCaMotor*>(sender()) &&
         static_cast<QCaMotor*>(sender())->isMoving() ) )
    return;

  if ( ! sender() || sender() == motors[Bragg1] ||
       sender() == Bragg1Enc || sender() == Bragg1EncLoss )
    calibratedMotors[Bragg1] =
        ! Bragg1EncLoss->get().toBool() &&
        qAbs( motors[Bragg1]->getUserPosition() - Bragg1Enc->get().toDouble() ) < 0.001 &&
        checkCalibrate(motors[Bragg1], travelRanges[Bragg1], 0.001);
  else if ( ! sender() || sender() == motors[Bragg2] ||
              sender() == Bragg2Enc || sender() == Bragg2EncLoss )
    calibratedMotors[Bragg2] =
        ! Bragg2EncLoss->get().toBool() &&
        qAbs( motors[Bragg2]->getUserPosition() - Bragg2Enc->get().toDouble() ) < 0.001 &&
        checkCalibrate(motors[Bragg2], travelRanges[Bragg2], 0.001);
  else if ( ! sender() || sender() == motors[Xdist] ||
         sender() == XdistEnc || sender() == XdistEncLoss )
    calibratedMotors[Xdist] =
        ! XdistEncLoss->get().toBool() &&
        qAbs( motors[Xdist]->getUserPosition() - XdistEnc->get().toDouble() ) < 0.001 &&
        checkCalibrate(motors[Xdist], travelRanges[Xdist], 0.01);
  else if ( ! sender() || sender() == motors[Z1]  )
    calibratedMotors[Z1] = checkCalibrate(motors[Z1], travelRanges[Z1], 0.01);
  else if ( ! sender() || sender() == motors[Z2]  )
    calibratedMotors[Z2] = checkCalibrate(motors[Z2], travelRanges[Z2], 0.01);
  else if ( ! sender() || sender() == motors[Tilt1]  )
    calibratedMotors[Tilt1] = checkCalibrate(motors[Tilt1], travelRanges[Tilt1], 0.01);
  else if ( ! sender() || sender() == motors[Tilt2]  )
    calibratedMotors[Tilt2] = checkCalibrate(motors[Tilt2], travelRanges[Tilt2], 0.01);
  else if ( ! sender() || sender() == motors[Bend1ob]  )
    calibratedMotors[Bend1ob] = checkCalibrate(motors[Bend1ob], travelRanges[Bend1ob], 0.01);
  else if ( ! sender() || sender() == motors[Bend2ob]  )
    calibratedMotors[Bend2ob] = checkCalibrate(motors[Bend2ob], travelRanges[Bend2ob], 0.01);
  else if ( ! sender() || sender() == motors[Bend1ib]  )
    calibratedMotors[Bend1ib] = checkCalibrate(motors[Bend1ib], travelRanges[Bend1ib], 0.01);
  else if ( ! sender() || sender() == motors[Bend2ib]  )
    calibratedMotors[Bend2ib] = checkCalibrate(motors[Bend2ib], travelRanges[Bend2ib], 0.01);

  emit calibrationChanged(isCalibrated());

}

bool Mono::isCalibrated() {
  bool allCalibrated = true;
  foreach(bool cal, calibratedMotors)
    allCalibrated &= cal;
  foreach(bool incal, incalibration)
    allCalibrated &= incal;
  return allCalibrated;
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
  if ( ! motors[Bragg1]->isConnected() || ! motors[Bragg2]->isConnected() ||
       motors[Bragg2]->isMoving() )
    return;
  _dBragg = motors[Bragg1]->getUserPosition()- motorAngle(energy(), 1, diffraction());
  //_dBragg = 1.0e6*delta*M_PI/180.0; //murad
  emit dBraggChanged(_dBragg);
}


void Mono::updateBragg2() {

  if ( ! motors[Bragg2]->isConnected() )
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
  if ( ! motors[Bragg1]->isConnected() ||
       ! motors[Xdist]->isConnected() )
    return;
  const double xDist = zSeparation() / tan(2*energy2bragg(energy(),diffraction())*M_PI/180);
  _dX = motors[Xdist]->getUserPosition() - xDist;
  emit dXChanged(_dX);
}


void Mono::updateZ1() {

  if ( ! motors[Z1]->isConnected() )
    return;

  double newZ1 = motors[Z1]->getUserPosition();
  if (motors[Z1]->isMoving())
    _inBeam = MOVING;
  else if ( qAbs(newZ1) <= 0.001 ) // 1 micron - unsertanty in Z
    _inBeam = INBEAM;
  else if ( newZ1 <= zOut )
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
  if ( ! motors[Z2]->isConnected() )
    return;
  emit dZChanged( _dZ = motors[Z2]->getUserPosition() - zSeparation() );
}


void Mono::updateTilt1() {
  if ( ! motors[Tilt1]->isConnected() )
    return;
  emit tilt1Changed(tilt1());
}


void Mono::updateTilt2() {
  if ( ! motors[Tilt2]->isConnected() )
    return;
  emit tilt2Changed(tilt2());
}


double Mono::bendR2X(double curvature, Mono::Motors mot) {

  double zero;
  double rt;
  switch (mot) {
  case Bend1ob:
    zero = bender1obZero;
    rt = bend1rt;
    break;
  case Bend1ib:
    zero = bender1ibZero;
    rt = bend1rt;
    break;
  case Bend2ob:
    zero = bender2obZero;
    rt = bend2rt;
    break;
  case Bend2ib:
    zero = bender2ibZero;
    rt = bend2rt;
    break;
  default:
    return 0;
  }

  if (curvature==0.0)
    return zero;

  double dt = curvature / ( rt * source2monoDistance );

  if (dt<0)
    return -dt + zero + benderGap/2.0;
  else
    return -dt + zero - benderGap/2.0;

}


double Mono::bendX2R(double pos, Mono::Motors mot) {

  double zero;
  double rt;
  switch (mot) {
  case Bend1ob:
    zero = bender1obZero;
    rt = bend1rt;
    break;
  case Bend1ib:
    zero = bender1ibZero;
    rt = bend1rt;
    break;
  case Bend2ob:
    zero = bender2obZero;
    rt = bend2rt;
    break;
  case Bend2ib:
    zero = bender2ibZero;
    rt = bend2rt;
    break;
  default:
    return 0;
  }

  double dt = -pos + zero;
  double curvature;
  if ( dt < -benderGap/2.0 )
    curvature = (dt + benderGap/2.0) / rt;
  else if ( dt > benderGap/2.0 )
    curvature = (dt - benderGap/2.0) / rt;
  else
    curvature = 0;
  return source2monoDistance * curvature;

}

void Mono::updateBend1ob() {
  if ( ! motors[Bend1ob]->isConnected() )
    return;
  b1ob = bendX2R(motors[Bend1ob]->getUserPosition(), Bend1ob);
  emit bend1obChanged(bend1ob());
}


void Mono::updateBend2ob() {
  if ( ! motors[Bend2ob]->isConnected() )
    return;
  b2ob = bendX2R(motors[Bend2ob]->getUserPosition(), Bend2ob);
  emit bend2obChanged(bend2ob());
}


void Mono::updateBend1ib() {
  if ( ! motors[Bend1ib]->isConnected() )
    return;
  b1ib = bendX2R(motors[Bend1ib]->getUserPosition(), Bend1ib);
  emit bend1ibChanged(bend1ib());
}


void Mono::updateBend2ib() {
  if ( ! motors[Bend2ib]->isConnected() )
    return;
  b2ib = bendX2R(motors[Bend2ib]->getUserPosition(), Bend2ib);
  emit bend2ibChanged(bend2ib());
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

  motors[Bragg1]->goUserPosition( motorAngle(enrg, 1, diff)
                                  + ( keepDBragg ? dBragg() * 180 / (1.0e06 * M_PI) : 0 ),
                                  QCaMotor::STARTED);
  motors[Bragg2]->goUserPosition( motorAngle(enrg, 2, diff),
                                  QCaMotor::STARTED);
  motors[Xdist]->goUserPosition( zSeparation() / tan(2*braggA*M_PI/180) +
                                 + ( keepDX ? dX() : 0 ),
                                 QCaMotor::STARTED);

  QTimer::singleShot(0, this, SLOT(updateBragg1()));
  QTimer::singleShot(0, this, SLOT(updateBragg2()));
  QTimer::singleShot(0, this, SLOT(updateX()));

}


void Mono::setDBragg(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Bragg1]->goUserPosition( motorAngle(energy(), 1, diffraction()) + val,
                                  QCaMotor::STARTED);
}


void Mono::setDX(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Xdist]->goUserPosition
      ( zSeparation() / tan(2*energy2bragg(energy(),diffraction())*M_PI/180) + val,
        QCaMotor::STARTED);
}


void Mono::setInBeam(bool val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Z1]->goUserPosition( val ? 0 : zOut);
}


void Mono::setZseparation(double val, bool keepDZ) {
  if ( ! isConnected() )
    return;
  emit zSeparationChanged(_zSeparation=val);
  setEnergy(energy(), diffraction());
  setDZ(keepDZ ? zTweak() : 0);
}

void Mono::setDZ(double val) {
  if ( ! isConnected() )
    return;
  motors[Z2]->goUserPosition( zSeparation()+val, QCaMotor::STARTED);
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


void Mono::setBend1ob(double val) {
  if ( ! isConnected() )
    return;
  double target = bendR2X(val, Bend1ob);
  motors[Bend1ob]->goUserPosition(target, QCaMotor::STARTED);
}


void Mono::setBend2ob(double val) {
  if ( ! isConnected() )
    return;
  double target = bendR2X(val, Bend2ob);
  motors[Bend2ob]->goUserPosition(target, QCaMotor::STARTED);
}


void Mono::setBend1ib(double val) {
  if ( ! isConnected() )
    return;
  double target = bendR2X(val, Bend1ib);
  motors[Bend1ib]->goUserPosition(target, QCaMotor::STARTED);
}


void Mono::setBend2ib(double val) {
  if ( ! isConnected() )
    return;
  double target = bendR2X(val, Bend2ib);
  motors[Bend2ib]->goUserPosition(target, QCaMotor::STARTED);
}


void Mono::stop() {
  foreach(QCaMotor * mot, motors)
    mot->stop();
}

void Mono::calibrate( const QList<Mono::Motors> & motors2calibrate ) {
  foreach(Motors motk, motors2calibrate)
    switch (motk) {
      case Bragg1 : QTimer::singleShot(0, this, SLOT(startCalBragg1()));
      case Bragg2 : QTimer::singleShot(0, this, SLOT(startCalBragg2()));
      case Xdist : QTimer::singleShot(0, this, SLOT(startCalXdist()));
      case Tilt1 : QTimer::singleShot(0, this, SLOT(startCalTilt1()));
      case Tilt2 : QTimer::singleShot(0, this, SLOT(startCalTilt2()));
      case Z1 : QTimer::singleShot(0, this, SLOT(startCalZ1()));
      case Z2 : QTimer::singleShot(0, this, SLOT(startCalZ2()));
      case Bend1ob : QTimer::singleShot(0, this, SLOT(startCalBend1ob()));
      case Bend1ib : QTimer::singleShot(0, this, SLOT(startCalBend1ib()));
      case Bend2ob : QTimer::singleShot(0, this, SLOT(startCalBend2ob()));
      case Bend2ib : QTimer::singleShot(0, this, SLOT(startCalBend2ib()));
    }
}


void Mono::calibrate(Mono::Motors motor) {

  incalibration[motor] = true;
  updateCalibration();
  QCaMotor * mot = motors[motor];
  const double currentPos = mot->getUserPosition();

  if (motor == Bragg1 && Bragg1EncLoss->get().toBool() ) {
    QList<ObjSig> os;
    os << ObjSig(Bragg1EncLoss, SIGNAL(valueChanged(QVariant)))
       << ObjSig(mot, SIGNAL(stopped()));
    mot->goLimit(1, QCaMotor::STARTED);
    qtWait(os);
  }
  if (motor == Bragg2 && Bragg2EncLoss->get().toBool() ) {
    QList<ObjSig> os;
    os << ObjSig(Bragg2EncLoss, SIGNAL(valueChanged(QVariant)))
       << ObjSig(mot, SIGNAL(stopped()));
    mot->goLimit(1, QCaMotor::STARTED);
    qtWait(os);
  }
  if (motor == Xdist && XdistEncLoss->get().toBool() ) {
    QList<ObjSig> os;
    os << ObjSig(XdistEncLoss, SIGNAL(valueChanged(QVariant)))
       << ObjSig(mot, SIGNAL(stopped()));
    mot->goLimit(1, QCaMotor::STARTED);
    qtWait(os);
  }

  mot->goLimit(-1, QCaMotor::STARTED);
  mot->wait_stop();

  if ( ( motor == Bragg1 && Bragg1EncLoss->get().toBool() ) ||
       ( motor == Bragg2 && Bragg2EncLoss->get().toBool() ) ||
       ( motor == Xdist && XdistEncLoss->get().toBool() ) ) {
       warn("Could not recover the encoder loss on motor \"" + mot->getPv() + "\".");
       incalibration[motor] = false;
       updateCalibration();
       return;
  }

  if (motor == Xdist)
    mot->goHome(-1, QCaMotor::STARTED); // BUG in EPICS on inverted axis
  else
    mot->goHome(1, QCaMotor::STARTED);
  mot->wait_stop();

  if ( motor == Bragg1 ||
       motor == Bragg2 ||
       motor == Xdist )
    QEpicsPv::set(mot->getPv()+":ENCODER_HMZ.PROC", 1, -1);

  mot->goUserPosition(currentPos, QCaMotor::STOPPED);
  mot->wait_stop();

  incalibration[motor] = false;
  updateCalibration();

}























