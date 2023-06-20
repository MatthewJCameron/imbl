#ifndef ShutterIAP_H
#define ShutterIAP_H

#include "component.h"
#include <qtpv.h>

class ShutterIAP : public Component {
  Q_OBJECT;

public:

  enum State {
    OPENED = 1,
    CLOSED = 0,
    BETWEEN = 2
  };

protected:
  State st;
  bool enabled;

  static QEpicsPv * openCommand;
  static QEpicsPv * closeCommand;

public:

  explicit ShutterIAP(QObject *parent = 0);
  ~ShutterIAP();

  inline State state() const{return st;};

public slots:

  bool open(bool wait=false);
  bool close(bool wait=false);

signals:

  void stateChanged(ShutterIAP::State);

protected slots:

  void updateState();
  void updateConnection();

};

#endif // ShutterIAP_H
