#ifndef SHUTTERFE_H
#define SHUTTERFE_H

#include "component.h"
#include <QTimer>
#include <qtpv.h>

class ShutterFE : public Component {
  Q_OBJECT;

public:

  enum State {
    OPENED = 1,
    CLOSED = 0,
    BETWEEN = 2
  };

  static const QString pvBaseName;
  static const int transitionTime = 2000; // msec

protected:

  static const int relaxTime;
  QTimer timer;

  State st;
  bool enabled;

  static QEpicsPv * opnSts;
  static QEpicsPv * clsSts;
  static QEpicsPv * opnCmd;
  static QEpicsPv * clsCmd;
  static QEpicsPv * enabledSts;
  static QEpicsPv * disabledSts;

public:

  explicit ShutterFE(QObject *parent = 0);
  ~ShutterFE();

  inline State state() const {return st;}
  inline bool isRelaxing() const {return timer.isActive();}
  inline bool isEnabled() const {return enabled;}

  static State stateS();
  static bool setOpenedS(bool opn, bool wait=false);

public slots:

  bool open(bool wait=false);
  bool close(bool wait=false);
  bool setOpened(bool opn, bool wait=false);
  bool toggle(bool wait=false);

signals:

  void opened();
  void closed();
  void stateChanged(ShutterFE::State st);
  void relaxChanged();
  void enabledChanged(bool);

protected slots:

  void updateState();
  void updateConnection();
  void updateEnabled();


};

#endif // SHUTTERFE_H
