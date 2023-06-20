#include "error.h"
#include "shutterIAP.h"
#include <QDebug>

QEpicsPv * ShutterIAP::openCommand = new QEpicsPv("SR08ID01ZEB02:SOFT_IN:B0");
QEpicsPv * ShutterIAP::closeCommand =  new QEpicsPv("SR08ID01ZEB02:SOFT_IN:B1");

ShutterIAP::ShutterIAP(QObject *parent) :
  Component("In-Air Pink Shutter", parent),
  st(BETWEEN)
{
  connect(openCommand, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(openCommand, SIGNAL(valueUpdated(QVariant)), SLOT(updateState()));
  connect(closeCommand, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(closeCommand, SIGNAL(valueUpdated(QVariant)), SLOT(updateState()));
  //QEpicsPv::setDebugLevel(1);
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
  } else if ( !(openCommand->get().toBool() ) && (closeCommand->get().toBool() ) ) {
    st = CLOSED;
    setDescription("Closed");
  } else {
    st = BETWEEN;
    setDescription("Between");
  }
  emit stateChanged(st);
}

bool ShutterIAP::open(bool wait) {
  if ( ! isConnected() )
    return false;
  closeCommand->set(0);
  qtWait(200);
  openCommand->set(1);
  qtWait(200);
  //updateState();
  return state() == OPENED ;
}

bool ShutterIAP::close(bool wait) {
  if ( ! isConnected() )
    return false;
  openCommand->set(0);
  qtWait(200);
  closeCommand->set(1);
  qtWait(200);
  //updateState();
  return state() == CLOSED;
}
