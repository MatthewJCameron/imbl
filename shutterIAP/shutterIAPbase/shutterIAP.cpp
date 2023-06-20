#include "error.h"
#include "shutterIAP.h"
#include <QDebug>

const int ShutterIAP::relaxTime = 300; // msec
QEpicsPv * ShutterIAP::openCommand = new QEpicsPv("SR08ID01ZEB02:SOFT_IN:B0");
QEpicsPv * ShutterIAP::closeCommand =  new QEpicsPv("SR08ID01ZEB02:SOFT_IN:B1");

ShutterIAP::ShutterIAP(QObject *parent) :
  Component("In-Air Pink Shutter", parent),
  enabled(false),
  st(BETWEEN)
{
  timer.setSingleShot(true);
  timer.setInterval(relaxTime);
  connect(openCommand, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(closeCommand, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  updateConnection();
}

ShutterIAP::~ShutterIAP() {
}

void ShutterIAP::updateConnection() {
  bool connection = openCommand->isConnected() && closeCommand->isConnected();
  setConnected(connection);
  if (isConnected()) {
    updateState();
  }
}

void ShutterIAP::updateState() {
  if ( (openCommand->get().toBool()) && !(closeCommand->get().toBool()) ) {
    st = OPENED;
    setDescription("Opened");
    emit opened();
  } else if !(openCommand->get().toBool()) && (closeCommand->get().toBool() ) {
    st = CLOSED;
    setDescription("Closed");
    emit closed();
  } else {
    st = BETWEEN;
    setDescription("Between");
    emit between();
  }
  emit stateChanged(st);
}

bool ShutterIAP::open(bool wait) {
  if ( ! isConnected() )
    return false;
  closeCommand->set(0);
  if (timer.isActive()) qtWait(&timer, SIGNAL(timeout()));
  openCommand->set(1);
  updateState()
  return state() == OPENED ;
}

bool ShutterIAP::close(bool wait) {
  if ( ! isConnected() )
    return false;
  openCommand->set(0);
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  closeCommand->set(1);
  updateState()
  return state() == CLOSED;
}