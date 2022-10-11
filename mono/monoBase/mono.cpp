#include "mono.h"
#include "error.h"
#include <qtpv.h>
#include "shutterFE.h"


static const double esinbq = 1.141394581356; // E * sin(Bragg) / sqrt (h*h + k*k + l*l) PRACTIC
//static const double esinbq = 1.1414666995476608627; // E * sin(Bragg) / sqrt (h*h + k*k + l*l) THEORY
static const double sqrt111 = sqrt(3);
static const double sqrt311 = sqrt(11);

double energy2bragg(double energy, Mono::Diffraction diff) {
  if (energy <= 0.0)
    return 0;
  return asin(esinbq * (diff==Mono::Si111 ? sqrt111 : sqrt311) / energy )
      * 180.0 / M_PI ;
}


const QString Mono::pvBaseName = "SR08ID01DCM01:";

const QHash<Mono::Motors,QCaMotor*> Mono::motors=Mono::init_motors();
const QHash < Mono::Motors, QPair<double,double> > Mono::travelRanges = Mono::init_ranges();
QEpicsPv * Mono::Bragg1Enc = new QEpicsPv(Mono::pvBaseName + "BRAGG1:ENCODER");
QEpicsPv * Mono::Bragg2Enc = new QEpicsPv(Mono::pvBaseName + "BRAGG2:ENCODER");
QEpicsPv * Mono::XdistEnc = new QEpicsPv(Mono::pvBaseName + "X:ENCODER");
QEpicsPv * Mono::Bragg1EncLoss = new QEpicsPv(Mono::pvBaseName + "BRAGG1:ELOSS");
QEpicsPv * Mono::Bragg2EncLoss = new QEpicsPv(Mono::pvBaseName + "BRAGG2:ELOSS");
QEpicsPv * Mono::XdistEncLoss = new QEpicsPv(Mono::pvBaseName + "X:ELOSS");

const QPair<double,double> Mono::energyRange = qMakePair<double,double>(16.0,195.0);
const double Mono::maxEnergy111 = 150.0; // keV
const double Mono::minEnergy311 = 31.0; // keV

const double Mono::standardZseparation = 25.0; // standard Z-separation of crystals; 
//the above was updated to 25 from 20 by Matthew on 11/10/2022 to reflect more common 25mm heights due to BCT
const double Mono::zOut = -45.0; // Z1 when the mono is out of the beam;
const double Mono::alpha = 14.75; // asymmetry angle (deg) "+" for 111 "-" for 311
// OLD CRYSTALS // const double Mono::alpha = -4.652; // asymmetry angle (deg)
const double Mono::source2monoDistance = 16.307;//m

const double Mono::benderGapH = 0.3; // halph bender gap
const double Mono::bend1rt = 58.112;
const double Mono::bend2rt = 62.2;
// const double Mono::bend1BraggCorrection = 0.00373; // deg(correction) / mm(tip travel)
// const double Mono::bend2BraggCorrection = 0.00326; // deg(correction) / mm(tip travel)
const double Mono::bend1BraggCorrection = 0.0; // no correction
const double Mono::bend2BraggCorrection = 0.0; // no correction


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
  _inBeam(BETWEEN),
  incalibration(false)
{

  foreach (Motors motk, motors.keys())
    calibratedMotors[motk]  = true;

  foreach(QCaMotor * mot, motors) {
    connect(mot, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
    connect(mot, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
    connect(mot, SIGNAL(changedConnected(bool)), SLOT(updateCalibration()));
    connect(mot, SIGNAL(changedMoving(bool)), SLOT(updateCalibration()));
    connect(mot, SIGNAL(changedUserPosition(double)), SLOT(updateCalibration()));
    connect(mot, SIGNAL(changedLoLimitStatus(bool)), SLOT(updateCalibration()));
    connect(mot, SIGNAL(changedHiLimitStatus(bool)), SLOT(updateCalibration()));
  }
  connect(motors[Z1],     SIGNAL(changedMoving(bool)), SLOT(updateZ1()));
  connect(motors[Bragg1], SIGNAL(changedUserPosition(double)), SLOT(updateDBragg()));
  connect(motors[Bend1ob], SIGNAL(changedUserPosition(double)), SLOT(updateDBragg()));
  connect(motors[Bend1ib], SIGNAL(changedUserPosition(double)), SLOT(updateDBragg()));
  connect(motors[Bragg2], SIGNAL(changedUserPosition(double)), SLOT(updateEnergy()));
  connect(motors[Bend2ob], SIGNAL(changedUserPosition(double)), SLOT(updateEnergy()));
  connect(motors[Bend2ib], SIGNAL(changedUserPosition(double)), SLOT(updateEnergy()));
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
  motret[Xdist]  = new QCaMotor(pvBaseName + "X");
  motret[Bragg1] = new QCaMotor(pvBaseName + "BRAGG1");
  motret[Bragg2] = new QCaMotor(pvBaseName + "BRAGG2");
  motret[Tilt1]  = new QCaMotor(pvBaseName + "TILT1");
  motret[Tilt2]  = new QCaMotor(pvBaseName + "TILT2");
  motret[Z1]     = new QCaMotor(pvBaseName + "Z1");
  motret[Z2]     = new QCaMotor(pvBaseName + "Z2");
  motret[Bend1ob]  = new QCaMotor(pvBaseName + "BENDER1OB");
  motret[Bend2ob]  = new QCaMotor(pvBaseName + "BENDER2OB");
  motret[Bend1ib]  = new QCaMotor(pvBaseName + "BENDER1IB");
  motret[Bend2ib]  = new QCaMotor(pvBaseName + "BENDER2IB");
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
  rangret[Bend1ob]  = qMakePair(-7.5,7.69);
  rangret[Bend2ob]  = qMakePair(-7.5,7.54);
  rangret[Bend1ib]  = qMakePair(-7.5,7.76);
  rangret[Bend2ib]  = qMakePair(-7.5,7.73);
  return rangret;
}




void Mono::wait_stop() {
  while ( isMoving() )
    qtWait(this, SIGNAL(motionChanged()));
}


void Mono::updateConnection() {
  bool con = true;
//  foreach(QCaMotor * mot, motors)
//    con &= mot->isConnected();

  con &= motors[Bragg1]->isConnected() &&
         motors[Bragg2]->isConnected() &&
         motors[Xdist]->isConnected();
  con &=
      Bragg1Enc->isConnected() && Bragg1EncLoss->isConnected() &&
      Bragg2Enc->isConnected() && Bragg2EncLoss->isConnected() &&
      XdistEnc->isConnected() && XdistEncLoss->isConnected();

  setConnected(con);

  if ( isConnected() ) {
    updateCalibration();
    updateDBragg();
    updateEnergy();
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
//  foreach(QCaMotor * mot, motors)
//    newMov |= mot->isMoving();
  newMov |= motors[Bragg1]->isMoving() &&
         motors[Bragg2]->isMoving() &&
         motors[Xdist]->isMoving();
  emit motionChanged(iAmMoving=newMov);
}


bool checkCalibrate (QCaMotor * mot, QPair<double,double> range, double allowance) {
  return
      mot->isConnected() &&
      mot->getUserPosition() >= range.first - allowance &&
      mot->getUserPosition() <= range.second + allowance &&
      ( ! mot->getLoLimitStatus() || mot->getUserPosition() <= range.first + allowance ) &&
      ( ! mot->getHiLimitStatus() || mot->getUserPosition() >= range.second - allowance );
}


void Mono::updateCalibration() {

  if ( static_cast<QCaMotor*>(sender()) &&
       static_cast<QCaMotor*>(sender())->isMoving() )
    return;

  if ( ! sender() || sender() == motors[Bragg1] ||
       sender() == Bragg1Enc || sender() == Bragg1EncLoss )
    calibratedMotors[Bragg1] =
        ! Bragg1EncLoss->get().toBool() &&
        qAbs( motors[Bragg1]->getUserPosition() - Bragg1Enc->get().toDouble() ) < 0.01 &&
        checkCalibrate(motors[Bragg1], travelRanges[Bragg1], 0.1);
  else if ( ! sender() || sender() == motors[Bragg2] ||
              sender() == Bragg2Enc || sender() == Bragg2EncLoss )
    calibratedMotors[Bragg2] =
        ! Bragg2EncLoss->get().toBool() &&
        qAbs( motors[Bragg2]->getUserPosition() - Bragg2Enc->get().toDouble() ) < 0.01 &&
        checkCalibrate(motors[Bragg2], travelRanges[Bragg2], 0.1);
  else if ( ! sender() || sender() == motors[Xdist] ||
         sender() == XdistEnc || sender() == XdistEncLoss )
    calibratedMotors[Xdist] =
        ! XdistEncLoss->get().toBool() &&
        qAbs( motors[Xdist]->getUserPosition() - XdistEnc->get().toDouble() ) < 0.01 &&
        checkCalibrate(motors[Xdist], travelRanges[Xdist], 0.1);
  else if ( ! sender() || sender() == motors[Z1]  )
    calibratedMotors[Z1] = checkCalibrate(motors[Z1], travelRanges[Z1], 0.1);
  else if ( ! sender() || sender() == motors[Z2]  )
    calibratedMotors[Z2] = checkCalibrate(motors[Z2], travelRanges[Z2], 0.1);
  else if ( ! sender() || sender() == motors[Tilt1]  )
    calibratedMotors[Tilt1] = checkCalibrate(motors[Tilt1], travelRanges[Tilt1], 0.1);
  else if ( ! sender() || sender() == motors[Tilt2]  )
    calibratedMotors[Tilt2] = checkCalibrate(motors[Tilt2], travelRanges[Tilt2], 0.1);
  else if ( ! sender() || sender() == motors[Bend1ob]  )
    calibratedMotors[Bend1ob] = checkCalibrate(motors[Bend1ob], travelRanges[Bend1ob], 0.1);
  else if ( ! sender() || sender() == motors[Bend2ob]  )
    calibratedMotors[Bend2ob] = checkCalibrate(motors[Bend2ob], travelRanges[Bend2ob], 0.1);
  else if ( ! sender() || sender() == motors[Bend1ib]  )
    calibratedMotors[Bend1ib] = checkCalibrate(motors[Bend1ib], travelRanges[Bend1ib], 0.1);
  else if ( ! sender() || sender() == motors[Bend2ib]  )
    calibratedMotors[Bend2ib] = checkCalibrate(motors[Bend2ib], travelRanges[Bend2ib], 0.1);

  emit calibrationChanged(isCalibrated());

}

bool Mono::isCalibrated() {
  bool allCalibrated = ! incalibration;
  foreach(bool cal, calibratedMotors)
    allCalibrated &= cal;
  return allCalibrated;
}


double Mono::motorAngle(double enrg, int crystal, Diffraction diff) {
  switch (crystal) {
  case 1:
    //return energy2bragg(enrg, diff) + (diff == Si111 ? -15.156015866278 : alpha); //TODO
    return energy2bragg(enrg, diff) + (diff == Si111 ? -20 : alpha); //TODO
  case 2:
    //return energy2bragg(enrg, diff) + (diff == Si111 ? 15.367584133722 : -1.0*alpha); //TODO
    return energy2bragg(enrg, diff) + (diff == Si111 ? 20 : -1.0*alpha); //TODO
  default:
    return 0;
  }
}


void Mono::updateDBragg() {
  if ( ! motors[Bragg1]->isConnected() || ! motors[Bragg2]->isConnected() ||
       motors[Bragg2]->isMoving() )
    return;
  _dBragg =
      motors[Bragg1]->getUserPosition()
      - motorAngle(energy(), 1, diffraction())
      - ( benderCorrection(Bend1ib) + benderCorrection(Bend1ob) ) / 2.0 ;
  //_dBragg = 1.0e6*delta*M_PI/180.0; //murad
  emit dBraggChanged(_dBragg);
}


void Mono::updateEnergy() {

  if ( ! motors[Bragg2]->isConnected() )
    return;

  const double mAngle = motors[Bragg2]->getUserPosition();
  double bAngle;
  //if ( mAngle >= alpha ) {
  if ( true ) { //DES//
    _diff = Si111;
    bAngle = mAngle - 20.0;  // - alpha; //TODO
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

  bAngle -= benderBraggCorrection() ;

  _energy = esinbq * ( diffraction()==Si111 ? sqrt111 : sqrt311 ) / sin(bAngle*M_PI/180);
  updateDBragg();
  updateX();
  emit energyChanged(_energy);

}


void Mono::updateX() {
  if ( ! motors[Bragg1]->isConnected() ||
       ! motors[Xdist]->isConnected() )
    return;
  const double xDist = zSeparation() / tan(2*energy2bragg(energy(),diffraction())*M_PI/180)
                       - 3.86217642235412456176 ;
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

  if (curvature==0.0)
    return 0;

  double rt;
  switch (mot) {
  case Bend1ob:
  case Bend1ib:
    rt = bend1rt;
    break;
  case Bend2ob:
  case Bend2ib:
    rt = bend2rt;
    break;
  default:
    return 0;
  }

  double dt = curvature * rt / source2monoDistance;
  return - dt - copysign(benderGapH, dt);

}


double Mono::bendX2R(Mono::Motors mot) {

  double rt;
  switch (mot) {
  case Bend1ob:
  case Bend1ib:
    rt = bend1rt;
    break;
  case Bend2ob:
  case Bend2ib:
    rt = bend2rt;
    break;
  default:
    return 0;
  }

  const double pos = motors[mot]->getUserPosition();
  double dt=0;
  if ( pos > benderGapH )
    dt = benderGapH - pos;
  else if ( pos < -benderGapH )
    dt = -benderGapH - pos;

  return dt * source2monoDistance / rt;

}


double Mono::benderCorrection(Mono::Motors mot) {

  double ct;
  switch (mot) {
  case Bend1ob:
  case Bend1ib:
    ct = bend1BraggCorrection;
    break;
  case Bend2ob:
  case Bend2ib:
    ct = bend2BraggCorrection;
    break;
  default:
    return 0;
  }

  const double pos = motors[mot]->getUserPosition();
  double dt=0;
  if ( pos > benderGapH )
    dt = benderGapH - pos;
  else if ( pos < -benderGapH )
    dt = -benderGapH - pos;

  return dt * ct;

}


double Mono::benderBraggCorrection() {
  return ( benderCorrection(Bend2ib) + benderCorrection(Bend2ob) ) / 2.0;
}



void Mono::updateBend1ob() {
  if ( ! motors[Bend1ob]->isConnected() )
    return;
  b1ob = bendX2R(Bend1ob);
  emit bend1obChanged(bend1ob());
}


void Mono::updateBend2ob() {
  if ( ! motors[Bend2ob]->isConnected() )
    return;
  b2ob = bendX2R(Bend2ob);
  emit bend2obChanged(bend2ob());
}


void Mono::updateBend1ib() {
  if ( ! motors[Bend1ib]->isConnected() )
    return;
  b1ib = bendX2R(Bend1ib);
  emit bend1ibChanged(bend1ib());
}


void Mono::updateBend2ib() {
  if ( ! motors[Bend2ib]->isConnected() )
    return;
  b2ib = bendX2R(Bend2ib);
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

  const double ddbragg = keepDBragg ? dBragg() : 0;
  const double ddx = keepDX ? dX() : 0;

  motors[Bragg1]->goUserPosition( motorAngle(enrg, 1, diff) + ddbragg
                                  + ( benderCorrection(Bend1ib) + benderCorrection(Bend1ob) ) / 2.0,
                                  QCaMotor::STARTED);
  motors[Bragg2]->goUserPosition( motorAngle(enrg, 2, diff) + benderBraggCorrection(), QCaMotor::STARTED);
  motors[Xdist]->goUserPosition( zSeparation() / tan(2*braggA*M_PI/180) + ddx - 3.86217642235412456176, QCaMotor::STARTED);

  QTimer::singleShot(0, this, SLOT(updateDBragg()));
  QTimer::singleShot(0, this, SLOT(updateEnergy()));
  QTimer::singleShot(0, this, SLOT(updateX()));

}




void Mono::setDBragg(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Bragg1]->goUserPosition(
        val
        + motorAngle(energy(), 1, diffraction())
        + ( benderCorrection(Bend1ib) + benderCorrection(Bend1ob) ) / 2.0,
        QCaMotor::STARTED);
}


void Mono::setDX(double val) {
  if ( ! isConnected() || isMoving() )
    return;
  motors[Xdist]->goUserPosition
      ( zSeparation() / tan(2*energy2bragg(energy(),diffraction())*M_PI/180) + val - 3.86217642235412456176,
        QCaMotor::STARTED);
}


void Mono::setInBeam(bool val) {
  if ( ! isConnected() || isMoving() )
    return;
  if ( ! ShutterFE::setOpenedS(false,true) ) {
    warn("Can't close the FE shutter."
         " Switching mono mode failed."
         " Try to repeat or do it manually.", this);
    return;
  }
  motors[Z1]->goUserPosition( val ? 0 : zOut);
}


void Mono::setZseparation(double val, bool keepZ, bool keepX) {
  if ( ! isConnected() )
    return;
  _zSeparation=val;

  if (keepX) updateX();
  else       setEnergy(energy(), diffraction(), true, true);

  if (keepZ) updateZ2();
  else       setDZ(zTweak());

  emit zSeparationChanged(_zSeparation);

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

  incalibration = true;
  updateCalibration();

  QHash<Mono::Motors, double> initialPositions;
  foreach(Motors motk, motors2calibrate)
    initialPositions[motk] = motors[motk]->getUserPosition();

  if ( ! ShutterFE::setOpenedS(false,true) ) {
    warn("Can't close the FE shutter."
         " Calibration of the mono failed."
         " Try to repeat or do it manually.", this);
    return;
  }

  if ( motors2calibrate.contains(Bragg1) && Bragg1EncLoss->get().toBool() ) {
    connect(Bragg1EncLoss, SIGNAL(valueChanged(QVariant)), motors[Bragg1], SLOT(stop()));
    motors[Bragg1]->goLimit(QCaMotor::POSITIVE, QCaMotor::STARTED);
  }
  if ( motors2calibrate.contains(Bragg2) && Bragg2EncLoss->get().toBool() ) {
    connect(Bragg2EncLoss, SIGNAL(valueChanged(QVariant)), motors[Bragg2], SLOT(stop()));
    motors[Bragg2]->goLimit(QCaMotor::POSITIVE, QCaMotor::STARTED);
  }
  if ( motors2calibrate.contains(Xdist) && XdistEncLoss->get().toBool() ) {
    connect(XdistEncLoss, SIGNAL(valueChanged(QVariant)), motors[Xdist], SLOT(stop()));
    motors[Xdist]->goLimit(QCaMotor::POSITIVE, QCaMotor::STARTED);
  }
  qtWait(200);
  if ( motors2calibrate.contains(Bragg1) ) {
    motors[Bragg1]->wait_stop();
    disconnect(Bragg1EncLoss, SIGNAL(valueChanged(QVariant)), motors[Bragg1], SLOT(stop()));
  }
  if ( motors2calibrate.contains(Bragg2) ) {
    motors[Bragg2]->wait_stop();
    disconnect(Bragg2EncLoss, SIGNAL(valueChanged(QVariant)), motors[Bragg2], SLOT(stop()));
  }
  if ( motors2calibrate.contains(Xdist) ) {
    motors[Xdist]->wait_stop();
    disconnect(XdistEncLoss, SIGNAL(valueChanged(QVariant)), motors[Xdist], SLOT(stop()));
  }

  foreach(Motors motk, motors2calibrate)
    motors[motk]->goLimit(QCaMotor::NEGATIVE, QCaMotor::STARTED);
  qtWait(200);
  foreach(Motors motk, motors2calibrate)
    motors[motk]->wait_stop();

  if ( ( motors2calibrate.contains(Bragg1) && Bragg1EncLoss->get().toBool() ) ||
       ( motors2calibrate.contains(Bragg2) && Bragg2EncLoss->get().toBool() ) ||
       ( motors2calibrate.contains(Xdist) && XdistEncLoss->get().toBool() ) ) {
    warn("Could not recover the encoder loss.");
    incalibration = false;
    updateCalibration();
    return;
  }

  foreach(Motors motk, motors2calibrate)
    if (motk == Xdist)
      motors[motk]->goHome(QCaMotor::NEGATIVE, QCaMotor::STARTED); // BUG in EPICS on inverted axis
    else
      motors[motk]->goHome(QCaMotor::POSITIVE, QCaMotor::STARTED);
  qtWait(200);
  foreach(Motors motk, motors2calibrate)
    motors[motk]->wait_stop();

  foreach(Motors motk, motors2calibrate)
    if ( motk == Bragg1 ||
         motk == Bragg2 ||
         motk == Xdist )
      QEpicsPv::set(motors[motk]->getPv()+":ENCODER_HMZ.PROC", 1, -1);

  foreach(Motors motk, motors2calibrate)
    motors[motk]->goUserPosition(initialPositions[motk], QCaMotor::STARTED);
  qtWait(200);
  foreach(Motors motk, motors2calibrate)
    motors[motk]->wait_stop();

  incalibration = false;
  updateCalibration();

}





















