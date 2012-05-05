#include "error.h"
#include "shutter1A.h"
#include <QDebug>

const int Shutter1A::relaxTime = 2500; // msec

QEpicsPv * Shutter1A::psOpenStatus =
    new QEpicsPv("SR08ID01PSS01:HU01A_BL_SHUTTER_OPEN_STS");
    //new QEpicsPv("SR08ID01PSS01:HU01A_PH_SHUTTER_OPEN_STS");
QEpicsPv * Shutter1A::psCloseStatus =
    new QEpicsPv("SR08ID01PSS01:HU01A_BL_SHUTTER_CLOSE_STS");
    //new QEpicsPv("SR08ID01PSS01:HU01A_PH_SHUTTER_CLOSE_STS");

QEpicsPv * Shutter1A::ssOpenStatus =
    new QEpicsPv("SR08ID01PSS01:HU01A_BL_SHT_OPEN_IND_STS");
    //new QEpicsPv("SR08ID01PSS01:HU01A_SF_SHUTTER_OPEN_STS");
QEpicsPv * Shutter1A::ssCloseStatus =
    new QEpicsPv("SR08ID01PSS01:HU01A_BL_SHT_CLOSE_IND_STS");
    //new QEpicsPv("SR08ID01PSS01:HU01A_SF_SHUTTER_CLOSE_STS");

QEpicsPv * Shutter1A::openCommand =
    new QEpicsPv("SR08ID01PSS01:HU01A_BL_SHUTTER_OPEN_CMD");
QEpicsPv * Shutter1A::closeCommand =
    new QEpicsPv("SR08ID01PSS01:HU01A_BL_SHUTTER_CLOSE_CMD");

QEpicsPv * Shutter1A::enabledStatus  =
    new QEpicsPv("SR08ID01PSS01:BL_EPS_BL_SHUT_ENABLE_STS");
QEpicsPv * Shutter1A::disabledStatus =
    new QEpicsPv("SR08ID01PSS01:BL_EPS_BL_SHUT_DISABLE_STS");


QEpicsPv * Shutter1A::mode1 = new QEpicsPv("SR08ID01PSS01:BL_OPMODE1_STS");
QEpicsPv * Shutter1A::mode2 = new QEpicsPv("SR08ID01PSS01:BL_OPMODE2_STS");
QEpicsPv * Shutter1A::mode3 = new QEpicsPv("SR08ID01PSS01:BL_OPMODE3_STS");

Shutter1A::Shutter1A(QObject *parent) :
  Component("1A shutter", parent),
  psst(BETWEEN),
  ssst(BETWEEN),
  md(INVALID),
  enabled(false)
{


  timer.setSingleShot(true);
  timer.setInterval(relaxTime);

  connect(mode1, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(mode2, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(mode3, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
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

  updateConnection();

}

Shutter1A::~Shutter1A() {
}

void Shutter1A::updateConnection() {
  bool connection =
      mode1->isConnected() && mode2->isConnected() && mode3->isConnected() &&
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
      ret = psState();
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
}



void Shutter1A::updateMode() {

  if ( ! isConnected() )
    return;

  if ( 1 !=
       mode1->get().toInt() +
       mode2->get().toInt() +
       mode3->get().toInt() )
    md = INVALID;
  else if ( mode1->get().toBool() )
    md = WHITE;
  else if ( mode2->get().toBool() )
    md = MRT;
  else if ( mode3->get().toBool() )
    md = MONO;

  emit modeChanged(md);

}

void Shutter1A::updateEnabled() {
  if ( ! isConnected() )
    return;
  bool newEnabled = enabledStatus->get().toBool() && ! disabledStatus->get().toBool();
  if (newEnabled != enabled)
    emit enabledChanged(enabled = newEnabled);
}

/*
Shutter1A::State Shutter1A::stateS() {
  Shutter1A sht;
  if ( ! sht.getReady(2000) ) {
    warn("Can't connect to the 1A shutter.", sht.objectName());
    return BETWEEN;
  }
  return sht.state();
}
*/

bool Shutter1A::open(bool wait) {
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  closeCommand->set(0);
  openCommand->set(1);
  if ( psst != OPENED && wait )
    return qtWait(this, SIGNAL(opened()), transitionTime);
  else
    return psst == OPENED ;
}

bool Shutter1A::close(bool wait) {
  if (timer.isActive())
    qtWait(&timer, SIGNAL(timeout()));
  openCommand->set(0);
  closeCommand->set(1);
  if ( psst != CLOSED && wait )
    return qtWait(this, SIGNAL(closed()), transitionTime);
  else
    return psst == CLOSED;
}

/*
bool Shutter1A::setOpenedS(bool opn, bool wait) {
  Shutter1A sht;
  if ( ! sht.getReady(2000) ) {
    warn("Can't connect to the 1A shutter.", sht.objectName());
    return BETWEEN;
  }
  return sht.setOpened(opn,wait);
}
*/

bool Shutter1A::setOpened(bool opn, bool wait) {
  if (opn) return open(wait);
  else     return close(wait);
}

bool Shutter1A::toggle(bool wait) {
  if (state()==OPENED) return close(wait);
  else                 return open(wait);
}

