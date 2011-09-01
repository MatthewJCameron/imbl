#include "mrtShutter.h"
#include <QDebug>

const QString MrtShutter::pvBaseName = "SR08ID01MRT01:";
const QHash<QString,QEpicsPv*> MrtShutter::pvs = MrtShutter::init_static();

MrtShutter::MrtShutter(QObject * parent) :
  Component("MRT shutter", parent),
  _exposure(0),
  _cycle(0),
  _repeats(0),
  _minCycle(0),
  _progress(0),
  _state(BETWEEN),
  _canStart(false),
  _valuesOK(false)
{

  foreach(QEpicsPv* pv, pvs)
    connect(pv, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));

  connect(pvs["SHUTTEROPEN_MONITOR"],         SIGNAL(valueChanged(QVariant)) , SLOT(updateState()));
  connect(pvs["SHUTTEROPEN_CMD"],             SIGNAL(valueChanged(QVariant)) , SLOT(updateState()));
  connect(pvs["EXPOSUREPERIOD_MONITOR"],      SIGNAL(valueChanged(QVariant)) , SLOT(updateExposure()));
  connect(pvs["EXPOSUREREPEATS_MONITOR"],     SIGNAL(valueChanged(QVariant)) , SLOT(updateRepeats()));
  connect(pvs["CYCLEPERIOD_MONITOR"],         SIGNAL(valueChanged(QVariant)) , SLOT(updateCycle()));
  connect(pvs["MINCYCLETIME_MONITOR"],        SIGNAL(valueChanged(QVariant)) , SLOT(updateMinCycle()));
  connect(pvs["EXPOSUREINPROGRESS_MONITOR"],  SIGNAL(valueChanged(QVariant)) , SLOT(updateProgress()));
  connect(pvs["REPETITIONSCOMPLETE_MONITOR"], SIGNAL(valueChanged(QVariant)) , SLOT(updateProgress()));
  connect(pvs["VALUESTATUS_MONITOR"],         SIGNAL(valueChanged(QVariant)) , SLOT(updateValuesOK()));
  connect(pvs["PSSENABLE_MONITOR"],           SIGNAL(valueChanged(QVariant)) , SLOT(updateCanStart()));

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
  _pvs["MINCYCLETIME_MONITOR"]        = new QEpicsPv(pvBaseName+"MINCYCLETIME_MONITOR");
  _pvs["EXPOSUREINPROGRESS_MONITOR"]  = new QEpicsPv(pvBaseName+"EXPOSUREINPROGRESS_MONITOR");
  _pvs["REPETITIONSCOMPLETE_MONITOR"] = new QEpicsPv(pvBaseName+"REPETITIONSCOMPLETE_MONITOR");
  _pvs["VALUESTATUS_MONITOR"]         = new QEpicsPv(pvBaseName+"VALUESTATUS_MONITOR");
  _pvs["PSSENABLE_MONITOR"]           = new QEpicsPv(pvBaseName+"PSSENABLE_MONITOR");

  return _pvs;

}


void MrtShutter::updateConnection() {
  bool con = true;
  foreach(QEpicsPv* pv, pvs)
    con &= pv->isConnected();
  setConnected(con);
  if (con) {
    updateExposure();
    updateCycle();
    updateRepeats();
    updateMinCycle();
    updateProgress();
    updateState();
    updateValuesOK();
    updateCanStart();
  }
}

void MrtShutter::updateCycle() {
  if (!isConnected())
    return;
  int newCycle = pvs["CYCLEPERIOD_MONITOR"]->get().toInt();
  if (newCycle != _cycle)
    emit cycleChanged(_cycle=newCycle);
}

void MrtShutter::updateExposure() {
  if (!isConnected())
    return;
  int newExposure = pvs["EXPOSUREPERIOD_MONITOR"]->get().toInt();
  if (newExposure != _exposure)
    emit exposureChanged(_exposure=newExposure);
}

void MrtShutter::updateRepeats() {
  if (!isConnected())
    return;
  int newRepeats = pvs["EXPOSUREREPEATS_MONITOR"]->get().toInt();
  if (newRepeats != _repeats)
    emit repeatsChanged(_repeats=newRepeats);
}

void MrtShutter::updateMinCycle() {
  if (!isConnected())
    return;
  int newMinCycle = pvs["MINCYCLETIME_MONITOR"]->get().toInt();
  if (newMinCycle != _minCycle)
    emit minCycleChanged(_minCycle=newMinCycle);
}

void MrtShutter::updateProgress() {
  if (!isConnected())
    return;
  int newProgress = pvs["EXPOSUREINPROGRESS_MONITOR"]->get().toBool()  ?
        pvs["REPETITIONSCOMPLETE_MONITOR"]->get().toInt() + 1  :  0;
  if (newProgress != _progress)
    emit progressChanged(_progress=newProgress);
}

void MrtShutter::updateState() {
  if (!isConnected())
    return;
  State newState;
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

void MrtShutter::start() {
  if (_canStart && _valuesOK)
    pvs["EXPOSURESTART_CMD"]->set(1);
}

void MrtShutter::stop() {
  pvs["EXPOSURESTART_CMD"]->set(0);
}

void MrtShutter::setExposure(int val) {
  pvs["EXPOSUREPERIOD_CMD"]->set(val);
}

void MrtShutter::setCycle(int val) {
  pvs["CYCLEPERIOD_MONITOR"]->set(val);
}

void MrtShutter::setRepeats(int val) {
  pvs["EXPOSUREREPEATS_CMD"]->set(val);
}





