#ifndef SHUTTERIS_H
#define SHUTTERIS_H

#include "component.h"
#include <qtpv.h>
#include <QTimer>

class ShutterIS : public Component {
  Q_OBJECT;

public:

  enum State {
    OPENED = 1,
    CLOSED = 0,
    BETWEEN = 2
  };

  static const QString pvBaseName;
  static const int transitionTime = 1000; // msec

protected:

  static const int relaxTime;
  QTimer timer;

  State st;
  bool enabled;

  static QEpicsPv * openStatus;
  static QEpicsPv * openCommand;
  static QEpicsPv * enabledStatus;

public:

  explicit ShutterIS(QObject *parent = 0);
  ~ShutterIS();

  inline State state() const{return st;};
  inline bool isRelaxing() const {return timer.isActive();}
  inline bool isEnabled() const {return enabled;}

public slots:

  bool open(bool wait=false);
  bool close(bool wait=false);
  bool toggle(bool wait=false);

signals:

  void stateChanged(ShutterIS::State);
  void relaxChanged();
  void opened();
  void closed();
  void enabledChanged(bool);

protected slots:

  void updateState();
  void updateConnection();
  void updateEnabled();

};

#endif // SHUTTERIS_H
