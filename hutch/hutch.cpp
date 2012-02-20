#include "error.h"
#include "hutch.h"
#include <QDebug>



Hutch::Hutch(Hutch::Hutches hu, QObject *parent) :
  Component("Hutch", parent),
  enabledPv(new QEpicsPv(this)),
  disabledPv(new QEpicsPv(this)),
  searchedPv(new QEpicsPv(this)),
  stackG(new QEpicsPv(this)),
  stackA(new QEpicsPv(this)),
  stackR(new QEpicsPv(this)),
  lDoorLocked(new QEpicsPv(this)),
  lDoorClosed(new QEpicsPv(this)),
  rDoorLocked(new QEpicsPv(this)),
  rDoorClosed(new QEpicsPv(this)),
  _stack(OFF),
  _enabled(false),
  _state(OPEN)
{

  QString baseName="SR08ID01PSS01:";
  switch (hu) {
    case H1A:
      baseName += "HU01A";
      setObjectName("Hutch 1A");
      break;
    case H1B:
      baseName += "HU01B";
      setObjectName("Hutch 1B");
      break;
    case H2A:
      baseName += "HU02A";
      setObjectName("Hutch 2A");
      break;
    case H2B:
      baseName += "HU02B";
      setObjectName("Hutch 2B");
      break;
    case TUN:
      baseName += "TUNNL";
      setObjectName("Tunnel");
      break;
    case H3A:
      baseName += "HU03A";
      setObjectName("Hutch 3A");
      break;
    case H3B:
      baseName += "HU03B";
      setObjectName("Hutch 3B");
      break;
  }

  enabledPv->setPV(baseName+"_ENABLE_STS");
  disabledPv->setPV(baseName+"_DISABLE_STS");
  searchedPv->setPV(baseName+"_SEARCHED_STS");
  stackG->setPV(baseName+"_STK_GREEN_STS");
  stackA->setPV(baseName+"_STK_AMBER_STS");
  stackR->setPV(baseName+"_STK_RED_STS");
  lDoorLocked->setPV(baseName+"_L_DOOR_LOCK_STS");
  lDoorClosed->setPV(baseName+"_L_DOOR_SW_STS");
  rDoorLocked->setPV(baseName+"_R_DOOR_LOCK_STS");
  rDoorClosed->setPV(baseName+"_R_DOOR_SW_STS");

  foreach(QEpicsPv * pv, findChildren<QEpicsPv*>() )
    connect(pv, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));

  connect(enabledPv, SIGNAL(valueUpdated(QVariant)), SLOT(updateEnabled()));
  connect(disabledPv, SIGNAL(valueUpdated(QVariant)), SLOT(updateEnabled()));
  connect(searchedPv, SIGNAL(valueUpdated(QVariant)), SLOT(updateState()));
  connect(lDoorLocked, SIGNAL(valueUpdated(QVariant)), SLOT(updateState()));
  connect(lDoorClosed, SIGNAL(valueUpdated(QVariant)), SLOT(updateState()));
  connect(rDoorLocked, SIGNAL(valueUpdated(QVariant)), SLOT(updateState()));
  connect(rDoorClosed, SIGNAL(valueUpdated(QVariant)), SLOT(updateState()));
  connect(stackG, SIGNAL(valueUpdated(QVariant)), SLOT(updateStack()));
  connect(stackR, SIGNAL(valueUpdated(QVariant)), SLOT(updateStack()));
  connect(stackA, SIGNAL(valueUpdated(QVariant)), SLOT(updateStack()));

  emit stackChanged(_stack);
  emit stateChanged(_state);
  emit enabledChanged(_enabled);

}


Hutch::~Hutch() {
  foreach(QEpicsPv * pv, findChildren<QEpicsPv*>())
    delete pv;
}

void Hutch::updateConnection() {
  bool connection = true;
  foreach(QEpicsPv * pv, findChildren<QEpicsPv*>())
    connection &= pv->isConnected();
  setConnected(connection);
  if (isConnected()) {
    updateStack();
    updateState();
    updateEnabled();
  }
}


void Hutch::updateStack() {
  if ( ! isConnected() )
    return;

  StackColor newStack;
  if ( stackR->get().toBool() )
    newStack = RED;
  else if ( stackA->get().toBool() )
    newStack = AMBER;
  else if ( stackG->get().toBool() )
    newStack = GREEN;
  else
    newStack = OFF;

  if (newStack != _stack)
    emit stackChanged(_stack=newStack);
}


void Hutch::updateState() {
  if ( ! isConnected() )
    return;

  State newState;
  if ( searchedPv->get().toBool() )
    newState = SEARCHED;
  else if ( lDoorLocked->get().toBool() && rDoorLocked->get().toBool() )
    newState = LOCKED;
  else if ( lDoorClosed->get().toBool() && rDoorClosed->get().toBool() )
    newState = CLOSED;
  else
    newState = OPEN;

  if (newState != _state)
    emit stateChanged(_state=newState);
}


void Hutch::updateEnabled() {
  if ( ! isConnected() )
    return;
  bool newEnab = enabledPv->get().toBool();
  if (newEnab != _enabled)
    emit enabledChanged(_enabled=newEnab);

}

