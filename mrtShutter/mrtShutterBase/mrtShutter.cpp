#include "mrtShutter.h"
#include "shutter1A.h"
#include <QDebug>

const QString MrtShutter::pvBaseName = "SR08ID01MRT01:";
Shutter1A * MrtShutter::shut1A = new Shutter1A();
const QHash<QString,QEpicsPv*> MrtShutter::pvs = MrtShutter::init_static();


MrtShutter::MrtShutter(QObject * parent) :
  Component("MRT shutter", parent),
  //dwellTimer(),
  _exposure(0),
  _cycle(0),
  _repeats(0),
  _progress(0),
  _state(CLOSED),
  _canStart(false),
  _valuesOK(false),
  _minRelax(0)
{

  //dwellTimer.setSingleShot(true);

  foreach(QEpicsPv* pv, pvs)
    connect(pv, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));

  connect(pvs["SHUTTEROPEN_MONITOR"],         SIGNAL(valueChanged(QVariant)) , SLOT(updateState()));
  connect(pvs["SHUTTEROPEN_CMD"],             SIGNAL(valueChanged(QVariant)) , SLOT(updateState()));
  connect(pvs["EXPOSUREPERIOD_MONITOR"],      SIGNAL(valueChanged(QVariant)) , SLOT(updateExposure()));
  connect(pvs["EXPOSUREREPEATS_MONITOR"],     SIGNAL(valueChanged(QVariant)) , SLOT(updateRepeats()));
  connect(pvs["CYCLEPERIOD_MONITOR"],         SIGNAL(valueChanged(QVariant)) , SLOT(updateCycle()));
  connect(pvs["EXPOSUREINPROGRESS_MONITOR"],  SIGNAL(valueChanged(QVariant)) , SLOT(updateProgress()));
  connect(pvs["REPETITIONSCOMPLETE_MONITOR"], SIGNAL(valueChanged(QVariant)) , SLOT(updateProgress()));
  connect(pvs["VALUESTATUS_MONITOR"],         SIGNAL(valueChanged(QVariant)) , SLOT(updateValuesOK()));
  connect(pvs["PSSENABLE_MONITOR"],           SIGNAL(valueChanged(QVariant)) , SLOT(updateCanStart()));
  connect(pvs["EXPOSURETRIGGERMODE_MONITOR"], SIGNAL(valueChanged(QVariant)) , SLOT(updateExposureMode()));
  connect(pvs["MINCYCLETIME_MONITOR"],        SIGNAL(valueChanged(QVariant)) , SLOT(updateMinRelax()));
  connect(pvs["BLADE2ACTIVATIONTIME_MONITOR"], SIGNAL(valueChanged(QVariant)) , SLOT(updateMinRelax()));
  connect(pvs["BLADE1DEACTIVATIONTIME_MONITOR"], SIGNAL(valueChanged(QVariant)) , SLOT(updateMinRelax()));
  connect(pvs["BLADE2DEACTIVATIONTIME_MONITOR"], SIGNAL(valueChanged(QVariant)) , SLOT(updateMinRelax()));

  //connect(&dwellTimer, SIGNAL(timeout()), SLOT(updateCanStart()));

  updateConnection();

}


const QHash<QString,QEpicsPv*> MrtShutter::init_static() {

  QHash<QString,QEpicsPv*> _pvs;

  _pvs["SHUTTEROPEN_CMD"]             = new QEpicsPv(pvBaseName+"SHUTTEROPEN_CMD");
  _pvs["SHUTTEROPEN_MONITOR"]         = new QEpicsPv(pvBaseName+"SHUTTEROPEN_MONITOR");
  _pvs["EXPOSURESTART_CMD"]           = new QEpicsPv(pvBaseName+"EXPOSURESTART_CMD");
  _pvs["EXPOSUREPERIOD_CMD"]          = new QEpicsPv(pvBaseName+"EXPOSUREPERIOD_CMD");
  _pvs["EXPOSUREPERIOD_MONITOR"]      = new QEpicsPv(pvBaseName+"EXPOSUREPERIOD_MONITOR");
  _pvs["EXPOSUREREPEATS_CMD"]         = new QEpicsPv(pvBaseName+"EXPOSUREREPEATS_CMD");
  _pvs["EXPOSUREREPEATS_MONITOR"]     = new QEpicsPv(pvBaseName+"EXPOSUREREPEATS_MONITOR");
  _pvs["CYCLEPERIOD_CMD"]             = new QEpicsPv(pvBaseName+"CYCLEPERIOD_CMD");
  _pvs["CYCLEPERIOD_MONITOR"]         = new QEpicsPv(pvBaseName+"CYCLEPERIOD_MONITOR");
  _pvs["EXPOSUREINPROGRESS_MONITOR"]  = new QEpicsPv(pvBaseName+"EXPOSUREINPROGRESS_MONITOR");
  _pvs["REPETITIONSCOMPLETE_MONITOR"] = new QEpicsPv(pvBaseName+"REPETITIONSCOMPLETE_MONITOR");
  _pvs["VALUESTATUS_MONITOR"]         = new QEpicsPv(pvBaseName+"VALUESTATUS_MONITOR");
  _pvs["PSSENABLE_MONITOR"]           = new QEpicsPv(pvBaseName+"PSSENABLE_MONITOR");
  _pvs["EXPOSURETRIGGERMODE_MONITOR"] = new QEpicsPv(pvBaseName+"EXPOSURETRIGGERMODE_MONITOR");
  _pvs["EXPOSURETRIGGERMODE_CMD"]     = new QEpicsPv(pvBaseName+"EXPOSURETRIGGERMODE_CMD");
  _pvs["SOFTWARETRIGGEREVENT_CMD"]    = new QEpicsPv(pvBaseName+"SOFTWARETRIGGEREVENT_CMD");
  _pvs["SOFTWARETRIGGEREVENT_MONITOR"]= new QEpicsPv(pvBaseName+"SOFTWARETRIGGEREVENT_MONITOR");
  _pvs["MINCYCLETIME_MONITOR"]        = new QEpicsPv(pvBaseName+"MINCYCLETIME_MONITOR");
  _pvs["BLADE2ACTIVATIONTIME_MONITOR"]= new QEpicsPv(pvBaseName+"BLADE2ACTIVATIONTIME_MONITOR");
  _pvs["BLADE1DEACTIVATIONTIME_MONITOR"]= new QEpicsPv(pvBaseName+"BLADE1DEACTIVATIONTIME_MONITOR");
  _pvs["BLADE2DEACTIVATIONTIME_MONITOR"]= new QEpicsPv(pvBaseName+"BLADE2DEACTIVATIONTIME_MONITOR");


  return _pvs;

}


void MrtShutter::updateConnection() {
  bool con = true;
  foreach(QEpicsPv* pv, pvs)
    con &= pv->isConnected();
  setConnected(con);
  if (con) {
    updateExposure();
    updateExposureMode();
    updateCycle();
    updateRepeats();
    updateMinRelax();
    updateProgress();
    updateState();
    updateValuesOK();
    updateCanStart();
  }
}

void MrtShutter::updateCycle() {
  if (!isConnected())
    return;
  double newCycle = /* 0.1 * */ pvs["CYCLEPERIOD_MONITOR"]->get().toInt();
  if (newCycle != _cycle)
    emit cycleChanged(_cycle=newCycle);
}

void MrtShutter::updateExposure() {
  if (!isConnected())
    return;
  double newExposure =/*  0.1 * */ pvs["EXPOSUREPERIOD_MONITOR"]->get().toInt();
  if (newExposure != _exposure)
    emit exposureChanged(_exposure=newExposure);
}

void MrtShutter::updateExposureMode() {
  if (!isConnected())
    return;
  ExposureMode newMode =
      (ExposureMode) pvs["EXPOSURETRIGGERMODE_MONITOR"]->get().toInt();
  if (newMode != _expMode)
    emit exposureModeChanged(_expMode=newMode);
}


void MrtShutter::updateRepeats() {
  if (!isConnected())
    return;
  int newRepeats = pvs["EXPOSUREREPEATS_MONITOR"]->get().toInt();
  if (newRepeats != _repeats)
    emit repeatsChanged(_repeats=newRepeats);
}

void MrtShutter::updateMinRelax() {
  if (!isConnected())
    return;
  double newMinRelax = /*0.1 * */(
        pvs["MINCYCLETIME_MONITOR"]->get().toInt() +
        pvs["BLADE2ACTIVATIONTIME_MONITOR"]->get().toInt() +
        pvs["BLADE1DEACTIVATIONTIME_MONITOR"]->get().toInt() +
        pvs["BLADE2DEACTIVATIONTIME_MONITOR"]->get().toInt() );
  if (newMinRelax != _minRelax)
    emit minRelaxChanged(_minRelax=newMinRelax);
}

void MrtShutter::updateProgress() {

  if (!isConnected())
    return;

  int newProgress  =  pvs["EXPOSUREINPROGRESS_MONITOR"]->get().toBool()  ?
                      pvs["REPETITIONSCOMPLETE_MONITOR"]->get().toInt() + 1   :   0;

  // if (newProgress != _progress) // commented out to avoid the situation when change in
  // EXPOSUREINPROGRESS_MONITOR  _and_ REPETITIONSCOMPLETE_MONITOR does not influence the progress.
  // It happens when EXPOSUREINPROGRESS_MONITOR turns to false.
  emit progressChanged(_progress=newProgress);
  updateState();

}

void MrtShutter::updateState() {
  if (!isConnected())
    return;
  State newState;
  if (progress())
    newState = EXPOSING;
  if ( pvs["SHUTTEROPEN_CMD"]->get() != pvs["SHUTTEROPEN_MONITOR"]->get() )
    newState = BETWEEN;
  else if ( pvs["SHUTTEROPEN_MONITOR"]->get().toInt() == 0 )
    newState = CLOSED;
  else
    newState = OPENED;
  if (newState != _state)
    emit stateChanged(_state=newState);
  updateCanStart();
}

void MrtShutter::updateCanStart() {
  bool newCan =
      isConnected()
      && valuesOK()
      && state() == CLOSED
      //&& pvs["PSSENABLE_MONITOR"]->get().toBool(); // What happens??? BUG!!
      //&& ! dwellTimer.isActive()
      && ! pvs["EXPOSUREINPROGRESS_MONITOR"]->get().toBool();
  if (newCan != _canStart)
    emit canStartChanged(_canStart=newCan);
}

void MrtShutter::updateValuesOK() {
  if (!isConnected())
    return;
  bool newOK = pvs["VALUESTATUS_MONITOR"]->get().toBool();
  if (newOK!=_valuesOK)
    emit valuesOKchanged(_valuesOK=newOK);
  updateCanStart();
}

void MrtShutter::setOpened(bool opn) {
  pvs["SHUTTEROPEN_CMD"]->set(opn ? 1 : 0);
}

void MrtShutter::trig(bool wait) {
  if ( ! isConnected() ||
       exposureMode() != SOFT ||
       ! progress() )
    return;
  pvs["SOFTWARETRIGGEREVENT_CMD"]->set(1);
  if (wait)
    qtWait(this, SIGNAL(progressChanged(int)), exposure() + cycle() );
}

void MrtShutter::start(bool beAwareOf1A) {
  // WARNING: BUG
  // Now there is no way I can determine if the safety shutter is opened - the status of the 1A shutter does not
  // reflect the behevioral differences in the fast and normal mode. the next if-block and beAware condition in the singleShot
  // are here to address the bug:
  // the command to open the safety shutter is sent and then relaxing for 1 sec to make sure it has opened. This waiting time
  // is redundant if the safety shutter was opened in prior.
  if (beAwareOf1A)
    shut1A->open(false);
  // WARNING: BUG
  // Perhaps not a bug, but a complex behaviour which may lead to a buggy situation.
  // If I call pvs["EXPOSURESTART_CMD"]->set(1) from within this function
  // there is a chance that the progressChanged signals will all be emited before this function returns what
  // makes it difficult to catch them to monitor the progress to detect the finish of the exposure.
  QTimer::singleShot(beAwareOf1A ? 1000 : 0, this, SLOT(actual_start()));
}

void MrtShutter::actual_start() {
  if ( ! canStart() )
    return;
  pvs["EXPOSURESTART_CMD"]->set(1);
}

void MrtShutter::stop() {
  updateProgress();
  emit progressChanged(_progress);
  pvs["EXPOSURESTART_CMD"]->set(0);
}

void MrtShutter::setExposure(double val) {
  double delta = cycle() - exposure();
  if ( delta < minRelax() )
    delta = minRelax();
  setCycle( val + delta );
  pvs["EXPOSUREPERIOD_CMD"]->set( (int)(/*10**/val) );
}

void MrtShutter::setExposureMode(ExposureMode val) {
  pvs["EXPOSURETRIGGERMODE_CMD"]->set( (int) val );
}


void MrtShutter::setCycle(double val) {
  pvs["CYCLEPERIOD_CMD"]->set( (int)(/*10**/val) );
}

void MrtShutter::setRepeats(int val) {
  pvs["EXPOSUREREPEATS_CMD"]->set(val);
}





