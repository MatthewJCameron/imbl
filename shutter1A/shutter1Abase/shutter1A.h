#ifndef SHUTTER1A_H
#define SHUTTER1A_H

#include "component.h"
#include <qtpv.h>
#include <QTimer>

class Shutter1A : public Component {
  Q_OBJECT;

public:

  enum Mode {
    INVALID=0,
    WHITE,
    MONO,
    MRT
  };

  enum State {
    OPENED = 1,
    CLOSED = 0,
    BETWEEN = 2
  };

  static const QString pvBaseName;
  static const int transitionTime = 5000; // msec

protected:

  static const int relaxTime;
  QTimer timer;

  State psst;
  State ssst;
  Mode md;
  bool enabled;

  static QEpicsPv * whiteMode;
  static QEpicsPv * monoMode;
  static QEpicsPv * mrtMode;

  static QEpicsPv * psOpenStatus;
  static QEpicsPv * psCloseStatus;
  static QEpicsPv * ssOpenStatus;
  static QEpicsPv * ssCloseStatus;

  static QEpicsPv * openCommand;
  static QEpicsPv * closeCommand;

  static QEpicsPv * enabledStatus;
  static QEpicsPv * disabledStatus;

public:

  explicit Shutter1A(QObject *parent = 0);
  ~Shutter1A();

  inline State psState() const {return psst;}
  inline State ssState() const {return ssst;}
  State state() const;
  inline Mode mode() const {return md;}
  inline bool isRelaxing() const {return timer.isActive();}
  inline bool isEnabled() const {return enabled;}

  //static State stateS();
  //static bool setOpenedS(bool opn, bool wait=false);

public slots:

  bool open(bool wait=false);
  bool close(bool wait=false);
  bool setOpened(bool opn, bool wait=false);
  bool toggle(bool wait=false);

signals:

  void psStateChanged(Shutter1A::State);
  void ssStateChanged(Shutter1A::State);
  void stateChanged(Shutter1A::State);
  void modeChanged(Shutter1A::Mode);
  void relaxChanged();
  void opened();
  void closed();
  void enabledChanged(bool);

protected slots:

  void updatePsState();
  void updateSsState();
  void updateState();
  void updateMode();
  void updateConnection();
  void updateEnabled();

};

#endif // SHUTTER1A_H
