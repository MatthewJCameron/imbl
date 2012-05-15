#ifndef VALVE_H
#define VALVE_H

#include "component.h"
#include <qtpv.h>


class Valve : public Component {
  Q_OBJECT;

public:
  enum State {
    INVALID = 1,
    CLOSED = 2,
    OPENED = 3,
    MOVING = 4
  };

private:

  static const QString pvTemplateState;
  static const QString pvTemplateCommand;
  State st;

protected:
  QEpicsPv * statePv;
  QEpicsPv * commandPv;

public:

  explicit Valve(int number, QObject *parent=0);
  ~Valve();

  inline State state() { return st; }

signals:
  void stateChanged(Valve::State);

public slots:
  void setOpened(bool opned);
  inline void open() {setOpened(true);}
  inline void close() {setOpened(false);}
  void toggle();

private slots:
  void updateConnection();
  void updateState();

};

#endif // VALVE_H
