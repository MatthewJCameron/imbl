#include "error.h"
#include "valve.h"
#include <QDebug>

const QString Valve::pvTemplateState = "SR08ID01EPS01:IGV%1_STS";
const QString Valve::pvTemplateCommand = "SR08ID01EPS01:IGV%1_OpenCloseCmd";

Valve::Valve(int number, QObject *parent) :
  Component("Valve" + QString::number(number), parent),
  statePv(new QEpicsPv(pvTemplateState.arg(number, 2, 10, QChar('0')), this)),
  commandPv(new QEpicsPv(pvTemplateCommand.arg(number, 2, 10, QChar('0')), this)),
  st(INVALID)
{
  connect(commandPv, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(statePv, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
  connect(statePv, SIGNAL(valueUpdated(QVariant)), SLOT(updateState()));
}


Valve::~Valve() {
  foreach(QEpicsPv * pv, findChildren<QEpicsPv*>())
    delete pv;
}

void Valve::updateConnection() {
  setConnected(statePv->isConnected()/* && commandPv->isConnected() */);
  if (isConnected())
    updateState();
}


void Valve::updateState() {
  bool ok;
  int new_st = statePv->get().toInt(&ok);
  if ( ! statePv->isConnected() | ! ok)
    st = INVALID;
  else
    st = (State) new_st;
  emit stateChanged(state());
}

void Valve::setOpened(bool opned) {
  if (!isConnected())
    return;
  commandPv->set( opned ? 2 : 1 );
}

void Valve::toggle() {
  if (state()==CLOSED)
    open();
  else if (state()==OPENED)
    close();
}


