#include "DEImono.h"
#include "error.h"

QCaMotor * DEImono::usMotor  = new QCaMotor("SR08ID01:MTR13B");
QCaMotor * DEImono::dsMotor = new QCaMotor("SR08ID01:MTR13C");
QCaMotor * DEImono::zMotor  = new QCaMotor("SR08ID01:MTR15A");

const double DEImono::totalCoeff = 1.977059; // (3/4)^(1/2) * h * c / d (keV, Angstr.)

const double DEImono::usArm =  235837.99 ; // mum
const double DEImono::usOffset = 17677.32 ; // mum
const double DEImono::dsArm =  235837.99 ; // mum
const double DEImono::dsOffset = 17677.32 ; // mum

const double DEImono::crystalLength = 80.0; // mm
const double DEImono::crystalGap = 5.0; // mm


DEImono::DEImono(QObject *parent) :
  Component("DEI mono", parent),
  iAmMoving(false),
  useDScrystal(false),
  _energy(10),
  _dsDisplacement(0),
  _zDisplacement(0)
{

  connect(usMotor, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
  connect(usMotor, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
  connect(usMotor, SIGNAL(changedUserPosition(double)), SLOT(updateUSposition()));

  connect(dsMotor, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
  connect(dsMotor, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
  connect(dsMotor, SIGNAL(changedUserPosition(double)), SLOT(updateDSdisplacement()));

  connect(zMotor, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
  connect(zMotor, SIGNAL(changedMoving(bool)), SLOT(updateMotion()));
  connect(zMotor, SIGNAL(changedUserPosition(double)), SLOT(updateZdisplacement()));

  updateConnection();

}


double DEImono::posFromEnergy (double energy, const double & off, const double & arm, double displacement) {
  if (energy <= totalCoeff) { // Should never happen
    warn( "Can't calculate crystal position for energy " + QString::number(energy) + "kEv.", "DEImono");
    energy = totalCoeff*1.1;
  }
  double sinbrg = totalCoeff / energy;
  if (displacement != 0.0)
    sinbrg = sin( asin(sinbrg) + displacement * 1.0E-06 ); // murad -> rad
  double tgbrg = sinbrg/sqrt(1-sinbrg*sinbrg);
  return off - arm * tgbrg;
}

double DEImono::zPosition(double energy, double displacement) {
  if (energy <= totalCoeff) { // Should never happen
    warn( "Can't calculate crystal position for energy " + QString::number(energy) + "kEv.", "DEImono");
    energy = totalCoeff*1.1;
  }
  double sinbrg = totalCoeff / energy;
  return crystalLength * sinbrg + crystalGap * sqrt(1-sinbrg*sinbrg) + displacement;
}

double DEImono::energy(double usPos) {
  double tgbrg = ( usOffset - usPos ) / usArm;
  double sinbrg = tgbrg / sqrt(1+tgbrg*tgbrg);
  if (sinbrg <=0) {
    warn( "Can't calculate the energy from the upstream position " + QString::number(usPos) + ".", "DEImono");
    return totalCoeff*1.1;
  }
  return totalCoeff / sinbrg;
}


void DEImono::wait_stop() {
  if (isMoving())
    qtWait(this, SIGNAL(stopped()));
}

void DEImono::updateConnection() {
  setConnected( usMotor->isConnected() && dsMotor->isConnected() && zMotor->isConnected() );
  if ( isConnected() ) {
    updateMotion();
    updateUSposition();
    updateDSdisplacement();
    updateZdisplacement();
  }
}

void DEImono::updateMotion() {
  bool newMov = usMotor->isMoving() || zMotor->isMoving()
      || (dsIsUsed() && dsMotor->isMoving());
  if (newMov != iAmMoving) {
    emit motionChanged(iAmMoving=newMov);
    if (!iAmMoving)
      emit stopped();
  }
}

void DEImono::updateUSposition() {
  double newEn = energy(usMotor->getUserPosition());
  if ( _energy != newEn )
    emit energyChanged(_energy=newEn);
  updateDSdisplacement();
  updateZdisplacement();
}

void DEImono::updateDSdisplacement() {
  double newDispl = 0;
  if (_energy <= totalCoeff) // Should never happen
    warn( "Can't calculate crystal displacement for energy " + QString::number(_energy) + "kEv.", this);
  else {
    double theoreticalAngle = asin( totalCoeff / _energy );
    double calculatedAngle = atan( ( usOffset - dsMotor->get() ) / usArm );
    newDispl = ( calculatedAngle - theoreticalAngle ) / 1.0E-06 ; // rad -> murad
  }
  if (newDispl != _dsDisplacement)
    emit dsDisplacementChanged(_dsDisplacement=newDispl);
}

void DEImono::updateZdisplacement() {
  double newDsipl = zMotor->get() - zPosition(_energy);
  if (newDsipl != _zDisplacement)
    emit zDisplacementChanged(_zDisplacement=newDsipl);
}


void DEImono::setEnergy(double val, bool keepDSdisplacement, bool keepZdisplacement) {
  usMotor->goUserPosition(usPosition(val), false);
  if (dsIsUsed())
    dsMotor->goUserPosition (dsPosition(val, keepDSdisplacement ? _dsDisplacement : 0.0), false);
  zMotor->goUserPosition (zPosition(val, keepZdisplacement ? _zDisplacement : 0.0), false);
}

void DEImono::useDS(bool use) {
  useDScrystal = use;
  if (use)
    setEnergy();
}

void DEImono::setDSdisplacement(double val) {
  _dsDisplacement = val;
  setEnergy();
}

void DEImono::setZdisplacement(double val) {
  _zDisplacement = val;
  setEnergy();
}























