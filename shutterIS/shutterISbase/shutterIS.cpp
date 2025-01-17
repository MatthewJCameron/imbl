#include "error.h"
#include "shutterIS.h"
#include <QDebug>

const int ShutterIS::relaxTime = 1000; // msec

QEpicsPv * ShutterIS::openStatus = new QEpicsPv("SR08ID01IS01:SHUTTEROPEN_MONITOR");
QEpicsPv * ShutterIS::openCommand = new QEpicsPv("SR08ID01IS01:SHUTTEROPEN_CMD");
QEpicsPv * ShutterIS::enabledStatus =  new QEpicsPv("SR08ID01IS01:SHUTTERENABLE_MONITOR");

ShutterIS::ShutterIS(QObject *parent) :
  Component("Imaging shutter", parent),
  enabled(false),
  st(BETWEEN)
{


  timer.setSingleShot(true);
  timer.setInterval(relaxTime);

  connect(openStatus, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(openCommand, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(enabledStatus, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(&timer, SIGNAL(timeout()), SIGNAL(relaxChanged()));

  connect(openStatus, SIGNAL(valueChanged(QVariant)), SLOT(updateState()));
  connect(enabledStatus, SIGNAL(valueChanged(QVariant)), SLOT(updateEnabled()));
  updateConnection();

}

ShutterIS::~ShutterIS() {
}

void ShutterIS::updateConnection() {
  bool connection =
      openStatus->isConnected() && openCommand->isConnected() && enabledStatus->isConnected();
  setConnected(connection);
  if (isConnected()) {
    updateState();
    updateEnabled();
    emit relaxChanged();
  }
}

void ShutterIS::updateState() {
if ( sender() == openStatus) {
    timer.start();
    emit relaxChanged();
  }

  if ( openStatus->get().toBool() ) {
    st = OPENED;
    setDescription("Opened");
    emit opened();
  } else {
    st = CLOSED;
    setDescription("Closed");
    emit closed();
  }
  emit stateChanged(st);

}

void ShutterIS::updateEnabled() {
  if ( ! isConnected() )
    return;
  bool newEnabled = enabledStatus->get().toBool();// && ! disabledStatus->get().toBool();
  emit enabledChanged(enabled = newEnabled);
}

bool ShutterIS::open(bool wait) {
  if ( ! isConnected() || ! isEnabled() )
    return false;
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  //closeCommand->set(0);
  openCommand->set(1);
  if ( state() != OPENED && wait )
    qtWait(this, SIGNAL(opened()), transitionTime);
  return state() == OPENED ;
}

bool ShutterIS::close(bool wait) {
  if ( ! isConnected() )
    return false;
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  //emit toggleRequested(0);
  openCommand->set(0);
  //closeCommand->set(1);
  if ( state() != CLOSED && wait )
    qtWait(this, SIGNAL(closed()), transitionTime);
  return state() == CLOSED;
}

bool ShutterIS::toggle(bool wait) {
  if (state()==CLOSED) return open(wait);
  else                 return close(wait);
}