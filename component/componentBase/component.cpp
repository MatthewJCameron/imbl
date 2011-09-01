#include "component.h"
#include "qtpv.h"
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

Component::Component(const QString & name, QObject * parent) :
  QObject(parent),
  iAmConnected(false),
  desc()
{
  setObjectName(name);
}

Component::Component(QObject * parent) :
  QObject(parent),
  iAmConnected(false),
  desc()
{}


bool Component::isConnected() const {
  return iAmConnected;
}

bool Component::getReady(int delay) {
  if ( ! iAmConnected )
    qtWait(this, SIGNAL(connectionChanged(bool)), delay);
  return iAmConnected;
}

const QString & Component::description() const {
  return desc;
}

void Component::setDescription(const QString & newDescription) {
  if (newDescription!=desc)
    emit descriptionChanged(desc=newDescription);
}

void Component::setConnected(bool newConnection) {
  if (newConnection != iAmConnected) {
    emit connectionChanged(iAmConnected = newConnection);
    if (!iAmConnected)
      setDescription("Disconnected.");
  }
}
