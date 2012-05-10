#ifndef VALVE_H
#define VALVE_H

#include "component.h"
#include <qtpv.h>


class Valve : public Component {
  Q_OBJECT;
private:
  static const QString pvTemplateStatus;
  static const QString pvTemplateCommand;
protected:
  //SR08ID01EPS01:IGV01_STS
  //SR08ID01EPS01:IGV01_OpenCloseCmd
  QEpicsPv * statusPv;
  QEpicsPv * commandPv;
public:
  explicit Valve(int number, QObject *parent=0);
  ~Valve();
  inline bool status() { return isConnected() && statusPv->get().toBool(); }
signals:
  void statusChanged(bool);
private slots:
  void updateConnection();
};

#endif // VALVE_H
