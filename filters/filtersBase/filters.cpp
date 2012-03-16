#include "filters.h"
#include "error.h"

QString materialName(Absorber::Material material){
  switch (material) {
  case Absorber::Empty: return "Empty";
  case Absorber::Beryllium: return "Be";
  case Absorber::Graphite: return "C";
  case Absorber::Aluminium: return "Al";
  case Absorber::Copper: return "Cu";
  case Absorber::Silver: return "Ag";
  case Absorber::Gold: return "Au";
  case Absorber::Molybdenum: return "Mo";
  case Absorber::Carborundum: return "SiC";
  default : return "";
  }
}

Absorber::Absorber(Material _material, double _thickness) {
  if (_thickness < 0.0 )
    throw_error("Negative thickness of a foil", "Filters/Absorber");
  _sandwich << Foil(_material, _thickness);
}

Absorber::Absorber(const QList<Foil> & sandw) :
  _sandwich(sandw)
{
  foreach (Foil foil, _sandwich)
    if (foil.second < 0.0 )
      throw_error("Negative thickness of a foil in the sandwich", "Filters/Absorber");
}


QString Absorber::description() const {
  QString tt;
  if (_sandwich.isEmpty()) {
    tt = "Invalid";
  } else {
    foreach(Foil foil , _sandwich) {
      tt += materialName(foil.first);
      if (foil.second != 0.0 && foil.first != Empty)
        tt += "(" + QString::number(foil.second) + ")";
      tt += " - ";
    }
    tt.chop(3);
  }
  return tt;
}





const double Paddle::position_accuracy = 0.1;

Paddle::Paddle(const QString &motorPV, const QList<Window> &_wins, double _incl, QObject *parent) :
  Component("Filter paddle driven by motor "+motorPV, parent),
  _motor(new QCaMotor(motorPV,this)),
  limitState(false),
  allWindows(_wins),
  currentWindow(-1),
  iAmMoving(false),
  incl(_incl)
{
  if (incl <= 0 || incl > M_PI/2.0)
    throw_error("Meaningless inclination " + QString::number(incl) + ".", this);
  connect(_motor, SIGNAL(changedUserPosition(double)), SLOT(updatePosiotion()));
  connect(_motor, SIGNAL(changedConnected(bool)), SLOT(updateConnection()));
  connect(_motor, SIGNAL(changedLoLimitStatus(bool)), SLOT(updateLimitState()));
  connect(_motor, SIGNAL(changedHiLimitStatus(bool)), SLOT(updateLimitState()));
  connect(_motor, SIGNAL(changedMoving(bool)), SLOT(updateMotionState()));
}

void Paddle::setWindow(int win) {
  if ( ! isConnected() )
    return;
  if (win < 0  ||  win >= allWindows.size()) {
    warn("Requested window does not exist in the paddle.", this);
    return;
  }
  _motor->goUserPosition(allWindows[win].first );
}


void Paddle::updateConnection() {
  setConnected(_motor->isConnected());
  if ( ! isConnected() )
    return;
  updatePosiotion();
  updateMotionState();
  updateLimitState();
}

void Paddle::updatePosiotion() {
  if ( ! isConnected() )
    return;
  double pos = _motor->get();
  int found_idx = -1;
  int idx=-1;
  while( ++idx < allWindows.size() && found_idx < 0 )
    if ( qAbs( allWindows[idx].first - pos ) <= Paddle::position_accuracy )
      found_idx = idx;
  if ( found_idx != currentWindow )
    emit windowChanged(currentWindow=found_idx);
}


void Paddle::updateMotionState() {
  if ( ! isConnected() )
    return;
  bool newMotion = _motor->isMoving();
  if ( newMotion != iAmMoving ) {
    emit motionStateChanged(iAmMoving=newMotion);
    if ( ! newMotion )
      emit stopped();
  }
}

void Paddle::updateLimitState() {
  if ( ! isConnected() )
    return;
  bool newState = _motor->getLoLimitStatus() || _motor->getHiLimitStatus();
  if ( newState != limitState )
    emit limitStateChanged(limitState=newState);
}






const QString Filters::pvBaseName="SR08ID01FR";
const QList<Paddle*> Filters::paddles = Filters::init();


Filters::Filters(QObject *parent) :
  Component(parent),
  iAmMoving(false)
{
  foreach(Paddle * paddle, paddles) {
    connect(paddle, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
    connect(paddle, SIGNAL(motionStateChanged(bool)), SLOT(updateMotionState()));
    connect(paddle, SIGNAL(windowChanged(int)), SLOT(updateWindows()));
  }
  updateConnection();
}

const QList<Paddle*> Filters::init() {

  QList<Paddle*> _paddles;

  _paddles << new Paddle(pvBaseName+"01:MTR01", QList<Paddle::Window>()
                         << Paddle::Window( 1.0,   Absorber(Absorber::Empty) )
                         << Paddle::Window( 59.9,  Absorber(Absorber::Beryllium, 0.2) )
                         << Paddle::Window( 124.7, Absorber(Absorber::Beryllium, 0.5) )
                         << Paddle::Window( 189.8, Absorber(Absorber::Graphite, 0.5) ),
                         M_PI/4.0 );

  _paddles << new Paddle(pvBaseName+"02:MTR01", QList<Paddle::Window>()
                         << Paddle::Window(1.0,    Absorber(Absorber::Empty) )
                         << Paddle::Window(68.5,   Absorber(Absorber::Carborundum, 0.25) )
                         << Paddle::Window(133.77, Absorber(Absorber::Carborundum, 0.5) )
                         << Paddle::Window(198.6,  Absorber( QList<Absorber::Foil>()
                                                             << Absorber::Foil(Absorber::Carborundum, 0.5)
                                                             << Absorber::Foil(Absorber::Carborundum, 0.5) ) ),
                         M_PI/4.0 );

  _paddles << new Paddle(pvBaseName+"03:MTR01", QList<Paddle::Window>()
                         << Paddle::Window(1.0,   Absorber(Absorber::Empty) )
                         << Paddle::Window(65.5,  Absorber(Absorber::Beryllium, 0.5) )
                         << Paddle::Window(130.9, Absorber(Absorber::Graphite, 1.0) )
                         << Paddle::Window(195.1, Absorber( QList<Absorber::Foil>()
                                                            << Absorber::Foil( Absorber::Graphite, 1.0)
                                                            << Absorber::Foil( Absorber::Aluminium, 0.2)
                                                            << Absorber::Foil( Absorber::Graphite, 1.0) ) ),
                         M_PI/4.0 );

  _paddles << new Paddle(pvBaseName+"04:MTR01", QList<Paddle::Window>()
                         << Paddle::Window(1.0,   Absorber(Absorber::Empty) )
                         << Paddle::Window(60.4,  Absorber(Absorber::Aluminium, 0.5) )
                         << Paddle::Window(125.9, Absorber(Absorber::Aluminium, 1.0) )
                         << Paddle::Window(190.7, Absorber(Absorber::Copper, 0.5) ),
                         M_PI/4.0);

  _paddles << new Paddle(pvBaseName+"05:MTR01", QList<Paddle::Window>()
                         << Paddle::Window(1.0,    Absorber(Absorber::Empty) )
                         << Paddle::Window(65.7,   Absorber(Absorber::Molybdenum, 0.125) )
                         << Paddle::Window(130.7,  Absorber(Absorber::Copper, 0.5) )
                         << Paddle::Window(195.64, Absorber(Absorber::Copper, 1.0) ),
                         M_PI/4.0 );

  return _paddles;

}


QList<int> Filters::windows() const {
  QList<int> wins;
  foreach(Paddle* paddle, paddles)
    wins << paddle->window();
  return wins;
}

bool Filters::isMissPositioned() const {
  bool misspos=false;
  foreach(Paddle* paddle, paddles)
    misspos |= paddle->window() < 0 ;
  return misspos;
}


void Filters::wait_stop() {
  if (!isConnected())
    return;
  QList<ObjSig> osS;
  osS
      << ObjSig(this, SIGNAL(connectionChanged(bool)))
      << ObjSig(this, SIGNAL(stopped()));
  if (iAmMoving)
    qtWait(osS);
}

void Filters::stop(bool wait) {
  if (!isConnected())
    return;
  foreach(Paddle* paddle, paddles)
    paddle->stop(false);
  if (wait)
    wait_stop();
}

void Filters::setWindows(const QList<int> &wins) {
  if (!isConnected())
    return;
  if ( wins.size() != paddles.size() ) {
    warn("Size of the requested set of windows is not equal to the number of paddles in the filter.", this);
    return;
  }
  for (int idx=0; idx < paddles.size(); idx++ )
    paddles[idx]->setWindow(wins[idx]);
}

void Filters::updateConnection() {
  bool con=true;
  foreach(Paddle* paddle, paddles)
    con &= paddle->isConnected();
  setConnected(con);
  if ( ! isConnected() )
    return;
  updateMotionState();
  updateWindows();
}

void Filters::updateWindows() {
  QList<Absorber::Foil> new_train;
  foreach(Paddle* paddle, paddles)
    foreach( Absorber::Foil foil, paddle->absorber().sandwich() ) {
      foil.second /= sin(paddle->inclination());
      new_train << foil;
    }
  if (new_train != _train)
    emit trainChanged( _train = new_train );
}

void Filters::updateMotionState() {
  if ( ! isConnected() )
    return;
  bool newMov=false;
  foreach(Paddle* paddle, paddles)
    newMov |= paddle->isMoving();
  if (newMov != iAmMoving)
    emit motionStateChanged(iAmMoving=newMov);
}






























