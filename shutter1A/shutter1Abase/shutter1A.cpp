#include "error.h"
#include "shutter1A.h"
#include <QDebug>

const int Shutter1A::relaxTime = 2500; // msec
const QString Shutter1A::pvBaseName = "SR08ID01PSS01:HU01A_BL_SHUTTER";
QEpicsPv * Shutter1A::opnSts = new QEpicsPv(Shutter1A::pvBaseName + "_OPEN_STS");
QEpicsPv * Shutter1A::clsSts = new QEpicsPv(Shutter1A::pvBaseName + "_CLOSE_STS");
QEpicsPv * Shutter1A::opnCmd = new QEpicsPv(Shutter1A::pvBaseName + "_OPEN_CMD");
QEpicsPv * Shutter1A::clsCmd = new QEpicsPv(Shutter1A::pvBaseName + "_CLOSE_CMD");


Shutter1A::Shutter1A(QObject *parent) :
  Component("1A shutter", parent),
  st(BETWEEN)
{


  timer.setSingleShot(true);
  timer.setInterval(relaxTime);

  connect(opnSts, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(clsSts, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(opnCmd, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(clsCmd, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(&timer, SIGNAL(timeout()), SIGNAL(relaxChanged()));

  connect(opnSts, SIGNAL(valueChanged(QVariant)), SLOT(updateState()));
  connect(clsSts, SIGNAL(valueChanged(QVariant)), SLOT(updateState()));

  updateConnection();

}

Shutter1A::~Shutter1A() {
}

void Shutter1A::updateConnection() {
  bool connection =
      opnSts->isConnected() && clsSts->isConnected() &&
      opnCmd->isConnected() && clsCmd->isConnected();
  setConnected(connection);
  if (isConnected()) {
    updateState();
    emit relaxChanged();
  }
}


void Shutter1A::updateState() {

  if ( sender() == opnSts || sender() == clsSts ) {
    timer.start();
    emit relaxChanged();
  }

  if ( clsSts->get().toBool() == opnSts->get().toBool() )  {
    st = BETWEEN;
    setDescription("Operation in proggress.");
  } else if ( opnSts->get().toBool() ) {
    st = OPENED;
    setDescription("Opened.");
    emit opened();
  } else {
    st = CLOSED;
    setDescription("Closed.");
    emit closed();
  }
  emit stateChanged(st);

}


Shutter1A::State Shutter1A::stateS() {
  Shutter1A sht;
  if ( ! sht.getReady(2000) ) {
    warn("Can't connect to the FE shutter.", sht.objectName());
    return BETWEEN;
  }
  return sht.state();
}

bool Shutter1A::open(bool wait) {
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  clsCmd->set(0);
  opnCmd->set(1);
  if ( st != OPENED && wait )
    return qtWait(this, SIGNAL(opened()), transitionTime);
  else
    return st == OPENED ;
}

bool Shutter1A::close(bool wait) {
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  opnCmd->set(0);
  clsCmd->set(1);
  if ( st != CLOSED && wait )
    return qtWait(this, SIGNAL(closed()), transitionTime);
  else
    return st == CLOSED;
}

bool Shutter1A::setOpenedS(bool opn, bool wait) {
  Shutter1A sht;
  if ( ! sht.getReady(2000) ) {
    warn("Can't connect to the FE shutter.", sht.objectName());
    return BETWEEN;
  }
  return sht.setOpened(opn,wait);
}

bool Shutter1A::setOpened(bool opn, bool wait) {
  if (opn) return open(wait);
  else     return close(wait);
}
