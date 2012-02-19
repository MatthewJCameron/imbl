#ifndef HUTCH_H
#define HUTCH_H

#include "component.h"
#include <qtpv.h>


class Hutch : public Component {
  Q_OBJECT;

public:

  static const QString pvBaseName;

  enum StackColor {
    OFF=0,
    GREEN,
    AMBER,
    RED
  } ;

  enum Hutches {
    H1A,
    H1B,
    H2A,
    H2B,
    TUN,
    H3A,
    H3B
  };

  enum State {
    OPEN,
    CLOSED,
    LOCKED,
    SEARCHED
  };

protected:

  QEpicsPv * enabledPv;
  QEpicsPv * disabledPv;
  QEpicsPv * searchedPv;
  QEpicsPv * stackG;
  QEpicsPv * stackA;
  QEpicsPv * stackR;
  QEpicsPv * lDoorLocked;
  QEpicsPv * lDoorClosed;
  QEpicsPv * rDoorLocked;
  QEpicsPv * rDoorClosed;

  StackColor _stack;
  bool _enabled;
  State _state;

public:

  explicit Hutch(Hutches hu, QObject *parent=0);
  ~Hutch();

  inline StackColor stack() {return _stack;}
  inline bool isEnabled() { return _enabled;}
  inline State state() { return _state;}

signals:

  void stackChanged(Hutch::StackColor col);
  void stateChanged(Hutch::State st);
  void enabledChanged(bool enab);

private slots:

  void updateConnection();
  void updateStack();
  void updateState();
  void updateEnabled();

};

#endif // HUTCH_H
