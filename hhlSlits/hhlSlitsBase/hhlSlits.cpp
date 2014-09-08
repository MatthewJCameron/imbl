#include "error.h"
#include "hhlSlits.h"

const QSizeF HhlSlits::fullbeam = QSizeF( 68.0, 4.1 );
const QString HhlSlits::pvBaseName = "SR08ID01SLW01:";

QCaMotor * HhlSlits::vOpen  = new QCaMotor(HhlSlits::pvBaseName + "VOPEN");
QCaMotor * HhlSlits::vPos   = new QCaMotor(HhlSlits::pvBaseName + "VPOS");
QCaMotor * HhlSlits::hLeft  = new QCaMotor(HhlSlits::pvBaseName + "LEFT");
QCaMotor * HhlSlits::hRight = new QCaMotor(HhlSlits::pvBaseName + "RIGHT");

const double  HhlSlits::vOpenCalibre = 0;
const double  HhlSlits::vPosCalibre = -2.2537;
const double  HhlSlits::hLeftCalibre = 0;
const double  HhlSlits::hRightCalibre = 0;



HhlSlits::HhlSlits(QObject *parent) :
  Component("High Heat Load Slits", parent),
  iAmMoving(false),
  lms(0),
  cl(0),
  cr(0),
  ch(0),
  cz(0)
{

  connect(vOpen, SIGNAL(changedConnected(bool)),  SLOT(updateConnection()) );
  connect(vOpen, SIGNAL(changedMoving(bool)),     SLOT(updateMotionState()) );
  connect(vOpen, SIGNAL(changedHiLimitStatus(bool)), SLOT(updateLimitState()) );
  connect(vOpen, SIGNAL(changedLoLimitStatus(bool)), SLOT(updateLimitState()) );
  connect(vOpen, SIGNAL(changedUserPosition(double)), SLOT(updateGeometry()) );

  connect(vPos, SIGNAL(changedConnected(bool)),  SLOT(updateConnection()) );
  connect(vPos, SIGNAL(changedMoving(bool)),     SLOT(updateMotionState()) );
  connect(vPos, SIGNAL(changedHiLimitStatus(bool)), SLOT(updateLimitState()) );
  connect(vPos, SIGNAL(changedLoLimitStatus(bool)), SLOT(updateLimitState()) );
  connect(vPos, SIGNAL(changedUserPosition(double)), SLOT(updateGeometry()) );

  connect(hLeft, SIGNAL(changedConnected(bool)),  SLOT(updateConnection()) );
  connect(hLeft, SIGNAL(changedMoving(bool)),     SLOT(updateMotionState()) );
  connect(hLeft, SIGNAL(changedHiLimitStatus(bool)), SLOT(updateLimitState()) );
  connect(hLeft, SIGNAL(changedLoLimitStatus(bool)), SLOT(updateLimitState()) );
  connect(hLeft, SIGNAL(changedUserPosition(double)), SLOT(updateGeometry()) );

  connect(hRight, SIGNAL(changedConnected(bool)),  SLOT(updateConnection()) );
  connect(hRight, SIGNAL(changedMoving(bool)),     SLOT(updateMotionState()) );
  connect(hRight, SIGNAL(changedHiLimitStatus(bool)), SLOT(updateLimitState()) );
  connect(hRight, SIGNAL(changedLoLimitStatus(bool)), SLOT(updateLimitState()) );
  connect(hRight, SIGNAL(changedUserPosition(double)), SLOT(updateGeometry()) );

  updateConnection();

}

void HhlSlits::wait_stop(){
  if (!isConnected())
    return;
  QList<ObjSig> osS;
  osS
      << ObjSig(this, SIGNAL(connectionChanged(bool)))
      << ObjSig(this, SIGNAL(stopped()));
  if (iAmMoving)
    qtWait(osS);
}

void HhlSlits::stop(bool wait) {
  if ( ! isConnected() || ! isMoving() )
    return;
  hLeft->stop();
  hRight->stop();
  vOpen->stop();
  vPos->stop();
  if (wait)
    wait_stop();
}

void HhlSlits::setGeometry(double _left, double _right, double _height, double _vcenter, bool wait) {
  if (!isConnected())
    return;
  stop(true);
  hLeft->goUserPosition(_left);
  hRight->goUserPosition(_right);
  vOpen->goUserPosition(_height);
  vPos->goUserPosition(_vcenter);
  if (wait) {
    qtWait(500);
    wait_stop();
  }
}

void HhlSlits::setTop(double val, bool wait) {
  QPair<double,double> hz = pn2oc(val, bottom());
  setGeometry(cl, cr, hz.first, hz.second, wait);
}

void HhlSlits::setBottom(double val, bool wait) {
  QPair<double,double> hz = pn2oc(top(), val);
  setGeometry(cl, cr, hz.first, hz.second, wait);
}

void HhlSlits::setHeight(double val, bool wait) {
  setGeometry(cl, cr, val, cz, wait);
}

void HhlSlits::setVCenter(double val, bool wait) {
  setGeometry(cl, cr, ch, val, wait);
}

void HhlSlits::setLeft(double val, bool wait) {
  setGeometry(val, cr, ch, cz, wait);
}

void HhlSlits::setRight(double val, bool wait) {
  setGeometry(cl, val, ch, cz, wait);
}

void HhlSlits::setWidth(double val, bool wait) {
  QPair<double,double> lr = oc2pn(val, hCenter());
  setGeometry(lr.first, lr.second, ch, cz, wait);
}

void HhlSlits::setHCenter(double val, bool wait) {
  QPair<double,double> lr = oc2pn(width(), val);
  setGeometry(lr.first, lr.second, ch, cz, wait);
}



void HhlSlits::updateConnection() {
  setConnected( vOpen->isConnected() && vPos->isConnected() &&
                hLeft->isConnected() && hRight->isConnected() );
  if ( isConnected() ) {
    updateGeometry();
    updateMotionState();
    updateLimitState();
  }
}

void HhlSlits::updateMotionState() {
  if (!isConnected())
    return;
  bool newMotion =
      vOpen->isMoving() || vPos->isMoving() ||
      hLeft->isMoving() || hRight->isMoving();
  if (newMotion != iAmMoving) {
    emit motionStateChanged(iAmMoving=newMotion);
    if (!newMotion)
      emit stopped();
  }

}

void HhlSlits::updateGeometry() {
  if (!isConnected())
    return;
  double newL = hLeft->get();
  double newR = hRight->get();
  double newH = vOpen->get();
  double newZ = vPos->get();
  if ( newL != cl || newR != cr ||
       newH != ch || newZ != cz)
    emit geometryChanged(cl=newL, cr=newR, ch=newH, cz=newZ);
}

void HhlSlits::updateLimitState() {
  if (!isConnected())
    return;
  Limits newLms;
  if (hLeft->getLoLimitStatus())  newLms |= LeftLo;
  if (hLeft->getHiLimitStatus())  newLms |= LeftHi;
  if (hRight->getLoLimitStatus())  newLms |= RightLo;
  if (hRight->getHiLimitStatus())  newLms |= RightHi;
  if (vOpen->getLoLimitStatus())  newLms |= HeightLo;
  if (vOpen->getHiLimitStatus())  newLms |= HeightHi;
  if (vPos->getLoLimitStatus())  newLms |= ZPosLo;
  if (vPos->getHiLimitStatus())  newLms |= ZPosHi;
  if (newLms != lms)
    emit limitStateChanged(lms=newLms);
}



