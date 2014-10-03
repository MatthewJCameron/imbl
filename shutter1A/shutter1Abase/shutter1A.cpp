#include "error.h"
#include "shutter1A.h"
#include <QDebug>

const int Shutter1A::relaxTime = 2500; // msec

QEpicsPv * Shutter1A::psOpenStatus =
    new QEpicsPv("SR08ID01PSS01:HU01A_PH_SHUTTER_OPEN_STS");
QEpicsPv * Shutter1A::psCloseStatus =
    new QEpicsPv("SR08ID01PSS01:HU01A_PH_SHUTTER_CLOSE_STS");

QEpicsPv * Shutter1A::ssOpenStatus =
    new QEpicsPv("SR08ID01PSS01:HU01A_SF_SHUTTER_OPEN_STS");
QEpicsPv * Shutter1A::ssCloseStatus =
    new QEpicsPv("SR08ID01PSS01:HU01A_SF_SHUTTER_CLOSE_STS");

QEpicsPv * Shutter1A::openCommand =
    new QEpicsPv("SR08ID01PSS01:HU01A_BL_SHUTTER_OPEN_CMD");
QEpicsPv * Shutter1A::closeCommand =
    new QEpicsPv("SR08ID01PSS01:HU01A_BL_SHUTTER_CLOSE_CMD");

QEpicsPv * Shutter1A::enabledStatus  =
    new QEpicsPv("SR08ID01PSS01:BL_EPS_BL_SHUT_ENABLE_STS");
QEpicsPv * Shutter1A::disabledStatus =
    new QEpicsPv("SR08ID01PSS01:BL_EPS_BL_SHUT_DISABLE_STS");


QEpicsPv * Shutter1A::whiteMode = new QEpicsPv("SR08ID01PSS01:HU01A_NOM_SHT_MOD_PERM_STS");
QEpicsPv * Shutter1A::monoMode = new QEpicsPv("SR08ID01PSS01:HU01A_MON_SHT_MOD_PERM_STS");
QEpicsPv * Shutter1A::mrtMode = new QEpicsPv("SR08ID01PSS01:HU01A_FST_SHT_MOD_PERM_STS");

Shutter1A::Shutter1A(QObject *parent) :
  Component("1A shutter", parent),
  psst(BETWEEN),
  ssst(BETWEEN),
  md(INVALID),
  enabled(false)
{


  timer.setSingleShot(true);
  timer.setInterval(relaxTime);

  connect(whiteMode, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(monoMode, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(mrtMode, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(psOpenStatus, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(psCloseStatus, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(ssOpenStatus, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(ssCloseStatus, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(openCommand, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(closeCommand, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(enabledStatus, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(disabledStatus, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(&timer, SIGNAL(timeout()), SIGNAL(relaxChanged()));

  connect(psOpenStatus, SIGNAL(valueChanged(QVariant)), SLOT(updatePsState()));
  connect(psCloseStatus, SIGNAL(valueChanged(QVariant)), SLOT(updatePsState()));
  connect(ssOpenStatus, SIGNAL(valueChanged(QVariant)), SLOT(updateSsState()));
  connect(ssCloseStatus, SIGNAL(valueChanged(QVariant)), SLOT(updateSsState()));
  connect(enabledStatus, SIGNAL(valueChanged(QVariant)), SLOT(updateEnabled()));
  connect(disabledStatus, SIGNAL(valueChanged(QVariant)), SLOT(updateEnabled()));

  connect(this, SIGNAL(ssStateChanged(Shutter1A::State)), SLOT(updateState()));
  connect(this, SIGNAL(psStateChanged(Shutter1A::State)), SLOT(updateState()));
  connect(this, SIGNAL(modeChanged(Shutter1A::Mode)), SLOT(updateState()));

  connect(whiteMode, SIGNAL(valueUpdated(QVariant)), SLOT(updateMode()));
  connect(monoMode, SIGNAL(valueUpdated(QVariant)), SLOT(updateMode()));
  connect(mrtMode, SIGNAL(valueUpdated(QVariant)), SLOT(updateMode()));

  updateConnection();

}

Shutter1A::~Shutter1A() {
}

void Shutter1A::updateConnection() {
  bool connection =
      whiteMode->isConnected() && monoMode->isConnected() && mrtMode->isConnected() &&
      psOpenStatus->isConnected() && psCloseStatus->isConnected() &&
      ssOpenStatus->isConnected() && ssCloseStatus->isConnected() &&
      openCommand->isConnected() && closeCommand->isConnected() &&
      enabledStatus->isConnected() && disabledStatus->isConnected();
  setConnected(connection);
  if (isConnected()) {
    updatePsState();
    updateSsState();
    updateMode();
    updateEnabled();
    emit relaxChanged();
  }
}

Shutter1A::State Shutter1A::state() const {
  State ret=BETWEEN;
  switch (mode()) {
    case MONO:
      ret = ssState();
      break;
    case WHITE:
      if ( ssState() == OPENED && psState() == OPENED )
        ret = OPENED;
      if ( ssState() == CLOSED && psState() == CLOSED )
        ret = CLOSED;
      else
        ret = BETWEEN;
      break;
    case MRT:
      ret = ssState();
      break;
    case INVALID:
      ret = BETWEEN;
      break;
  }
  return ret;
}


void Shutter1A::updatePsState() {

  if ( ! isConnected() )
    return;

  if ( sender() == psOpenStatus || sender() == psCloseStatus ) {
    timer.start();
    emit relaxChanged();
  }

  if ( psCloseStatus->get().toBool() == psOpenStatus->get().toBool() )
    psst = BETWEEN;
  else if ( psOpenStatus->get().toBool() )
    psst = OPENED;
  else
    psst = CLOSED;
  emit psStateChanged(psst);

}

void Shutter1A::updateSsState() {

  if ( ! isConnected() )
    return;

  if ( sender() == ssOpenStatus || sender() == ssCloseStatus ) {
    timer.start();
    emit relaxChanged();
  }

  if ( ssCloseStatus->get().toBool() == ssOpenStatus->get().toBool() )
    ssst = BETWEEN;
  else if ( ssOpenStatus->get().toBool() )
    ssst = OPENED;
  else
    ssst = CLOSED;
  emit ssStateChanged(ssst);

}


void Shutter1A::updateState() {
  QString desc;
  desc+="PS: ";
  switch (psState()){
    case OPENED: desc+="opened"; break;
    case CLOSED: desc+="closed"; break;
    case BETWEEN: desc+="between"; break;
  }
  desc+="SS: ";
  switch (ssState()){
    case OPENED: desc+="opened"; break;
    case CLOSED: desc+="closed"; break;
    case BETWEEN: desc+="between"; break;
  }
  setDescription(desc);
  emit stateChanged(state());
  if (state() == OPENED)
    emit opened();
  else if (state() == CLOSED)
    emit closed();
}



void Shutter1A::updateMode() {

  if ( ! isConnected() )
    return;

  if ( 1 !=
       whiteMode->get().toInt() +
       monoMode->get().toInt() +
       mrtMode->get().toInt() )
    md = INVALID;
  else if ( whiteMode->get().toBool() )
    md = WHITE;
  else if ( mrtMode->get().toBool() )
    md = MRT;
  else if ( monoMode->get().toBool() )
    md = MONO;

  emit modeChanged(md);

}

void Shutter1A::updateEnabled() {
  if ( ! isConnected() )
    return;
  bool newEnabled = enabledStatus->get().toBool() && ! disabledStatus->get().toBool();
  emit enabledChanged(enabled = newEnabled);
}



bool Shutter1A::open(bool wait) {
  if ( ! isConnected() || ! isEnabled() )
    return false;
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  emit toggleRequested(1);
  closeCommand->set(0);
  openCommand->set(1);
  if ( state() != OPENED && wait )
    qtWait(this, SIGNAL(opened()), transitionTime);
  return state() == OPENED ;
}

bool Shutter1A::close(bool wait) {
  if ( ! isConnected() )
    return false;
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  emit toggleRequested(0);
  openCommand->set(0);
  closeCommand->set(1);
  if ( state() != CLOSED && wait )
    qtWait(this, SIGNAL(closed()), transitionTime);
  return state() == CLOSED;
}



bool Shutter1A::setOpened(bool opn, bool wait) {
  if (opn) return open(wait);
  else     return close(wait);
}

bool Shutter1A::toggle(bool wait) {
  if (state()==CLOSED) return open(wait);
  else                 return close(wait);
}

