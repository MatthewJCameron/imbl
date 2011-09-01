#include "error.h"
#include "shutterFE.h"
#include <QDebug>

const int ShutterFE::relaxTime = 5000; // msec
const QString ShutterFE::pvBaseName = "SR08ID01PSS01:FE_SHUTTER";
QEpicsPv * ShutterFE::opnSts = new QEpicsPv(ShutterFE::pvBaseName + "_OPEN_STS");
// BUG !!!
// SR08ID01PSS01:FE_SHUTTER_CLOSE_STS PV does not exist
QEpicsPv * ShutterFE::clsSts = new QEpicsPv(ShutterFE::pvBaseName + "_CLOSE_STS");
QEpicsPv * ShutterFE::opnCmd = new QEpicsPv(ShutterFE::pvBaseName + "_OPEN_CMD");
QEpicsPv * ShutterFE::clsCmd = new QEpicsPv(ShutterFE::pvBaseName + "_CLOSE_CMD");


ShutterFE::ShutterFE(QObject *parent) :
  Component("Fron-end shutter", parent),
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

ShutterFE::~ShutterFE() {
}

void ShutterFE::updateConnection() {
  bool connection =
      opnSts->isConnected() && /* clsSts->isConnected() && */ // see BUG above
      opnCmd->isConnected() && clsCmd->isConnected();
  setConnected(connection);
  if (isConnected()) {
    updateState();
    emit relaxChanged();
  }
}


void ShutterFE::updateState() {

  if ( sender() == opnSts || sender() == clsSts ) {
    timer.start();
    emit relaxChanged();
  }

  if ( /* clsSts->get().toBool() == opnSts->get().toBool() */ false )  { // see BUG above
    st = BETWEEN;
    setDescription("Operation in proggress");
  } else if ( opnSts->get().toBool() ) {
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


ShutterFE::State ShutterFE::stateS() {
  ShutterFE sht;
  if ( ! sht.getReady(2000) ) {
    warn("Can't connect to the FE shutter.", sht.objectName());
    return BETWEEN;
  }
  return sht.state();
}

bool ShutterFE::open(bool wait) {
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  clsCmd->set(0);
  opnCmd->set(1);
  if ( st != OPENED && wait )
    return qtWait(this, SIGNAL(opened()), transitionTime);
  else
    return st == OPENED ;
}

bool ShutterFE::close(bool wait) {
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  opnCmd->set(0);
  clsCmd->set(1);
  if ( st != CLOSED && wait )
    return qtWait(this, SIGNAL(closed()), transitionTime);
  else
    return st == CLOSED;
}

bool ShutterFE::setOpenedS(bool opn, bool wait) {
  ShutterFE sht;
  if ( ! sht.getReady(2000) ) {
    warn("Can't connect to the FE shutter.", sht.objectName());
    return BETWEEN;
  }
  return sht.setOpened(opn,wait);
}

bool ShutterFE::setOpened(bool opn, bool wait) {
  if (opn) return open(wait);
  else     return close(wait);
}
