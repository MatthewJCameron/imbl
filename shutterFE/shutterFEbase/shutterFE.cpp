#include "error.h"
#include "shutterFE.h"
#include <QDebug>
#include <QMessageBox>

const int ShutterFE::relaxTime = 5000; // msec
const QString ShutterFE::pvBaseName = "SR08ID01PSS01:FE_SHUTTER";
QEpicsPv * ShutterFE::opnSts = new QEpicsPv(ShutterFE::pvBaseName + "_OPEN_STS");
QEpicsPv * ShutterFE::clsSts = new QEpicsPv(ShutterFE::pvBaseName + "_CLOSED_STS");
QEpicsPv * ShutterFE::opnCmd = new QEpicsPv(ShutterFE::pvBaseName + "_OPEN_CMD");
QEpicsPv * ShutterFE::clsCmd = new QEpicsPv(ShutterFE::pvBaseName + "_CLOSE_CMD");
QEpicsPv * ShutterFE::enabledSts  = new QEpicsPv("SR08ID01PSS01:FES_EPS_ENABLE_STS");
QEpicsPv * ShutterFE::disabledSts = new QEpicsPv("SR08ID01PSS01:FES_EPS_DISABLE_STS");
QEpicsPv * ShutterFE::mono1ZRBV = new QEpicsPv("SR08ID01DCM01:Z1.RBV");


ShutterFE::ShutterFE(QObject *parent) :
  Component("Front-end shutter", parent),
  st(BETWEEN),
  enabled(false),
  paddle4isNoneBool(true),
  paddle5isNoneBool(true),
  mono1isInBool(true),
  expanderisInBool(true),
  bctTableisInBool(true),
  shutterisMonoBool(false)
{

  timer.setSingleShot(true);
  timer.setInterval(relaxTime);

  connect(opnSts, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(clsSts, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(opnCmd, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(clsCmd, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(enabledSts, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(disabledSts, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(&timer, SIGNAL(timeout()), SIGNAL(relaxChanged()));

  connect(opnSts, SIGNAL(valueChanged(QVariant)), SLOT(updateState()));
  connect(clsSts, SIGNAL(valueChanged(QVariant)), SLOT(updateState()));
  connect(enabledSts, SIGNAL(valueChanged(QVariant)), SLOT(updateEnabled()));
  connect(disabledSts, SIGNAL(valueChanged(QVariant)), SLOT(updateEnabled()));

  updateConnection();

}

ShutterFE::~ShutterFE() {
}

void ShutterFE::updateConnection() {
  bool connection =
      opnSts->isConnected() && clsSts->isConnected() &&
      opnCmd->isConnected() && clsCmd->isConnected() &&
      enabledSts->isConnected() && disabledSts->isConnected();
  setConnected(connection);
  if (isConnected()) {
    updateState();
    updateEnabled();
    emit relaxChanged();
  }
}


void ShutterFE::updateState() {

  if ( sender() == opnSts || sender() == clsSts ) {
    timer.start();
    emit relaxChanged();
  }

  if ( clsSts->get().toBool() == opnSts->get().toBool() )  {
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

void ShutterFE::updateEnabled() {
  if ( ! isConnected() )
    return;
  bool newEnabled = enabledSts->get().toBool() && ! disabledSts->get().toBool();
  emit enabledChanged(enabled = newEnabled);
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
  if ( ! isConnected() )
    return false;
  if ( ! isEnabled() )
    return state() == OPENED;
  if (not shutterisMonoBool) {
    if (expanderisInBool){
      QMessageBox::warning(0, "Expander is in place while shutter mode is white","Mode is not Mono and expander is NOT OUT OF BEAM. SWAP TO MONO SHUTTER MODE AND REMOVE EXPANDER.");
      return false;
      }
    if (bctTableInBool){
      QMessageBox::warning(0, "BCT table is in place while shutter mode is white","Mode is not Mono and bct table is NOT OUT OF BEAM. SWAP TO MONO SHUTTER MODE AND REMOVE BCT TABLE.");
      return false;
      }
    if(mono1isInBool){
      QMessageBox::warning(0, "Mono crystal 1 is IN while shutter mode is white","Mode is not Mono and Crystal 1 is NOT OUT OF BEAM. REMOVE MONOCHROMATOR FROM BEAM.");
      return false;  
      }
  }
  if ((mono1isInBool) && (paddle4isNoneBool) && (paddle5isNoneBool) ){
    QMessageBox::warning(0, "Mono crystal 1 is IN while Paddle 4 and 5 are None","Crystal 1 is NOT OUT OF BEAM while Paddle 4 and 5 are NONE. REMOVE MONOCHROMATOR FROM BEAM OR REINTRODUCE FILTRATION PRIOR TO MONO CRYSTAL.");
    return false;
  }
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
  if ( ! isConnected() )
    return false;
  //if ( ! isEnabled() )
  //  return state() == CLOSED;
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

bool ShutterFE::toggle(bool wait) {
  if (state()==CLOSED) return open(wait);
  else                 return close(wait);
}

void ShutterFE::Paddle4isNone(bool newValue){
  paddle4isNoneBool=newValue;
}

void ShutterFE::Paddle5isNone(bool newValue){
  paddle5isNoneBool=newValue;
}

void ShutterFE::mono1isIn(bool newValue){
  mono1isInBool=newValue;
}

void ShutterFE::expanderisIn(bool newValue){
  expanderisInBool=newValue;
}

void ShutterFE::bctTableisIn(bool newValue){
  bctTableInBool=newValue;
}

void ShutterFE::shutterisMono(bool newValue){
  shutterisMonoBool=newValue;
}