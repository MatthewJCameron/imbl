#ifndef QIMBL_H
#define QIMBL_H

#include <QtGui/QMainWindow>
#include <QLabel>
#include "shutterFE.h"
#include "mrtShutter.h"
#include "hhlSlitsGui.h"
#include "filtersGui.h"
#include "monoGui.h"
#include "hutch.h"
#include "valuebar.h"
#include <qtpv.h>

namespace Ui {
class Qimbl;
}

class Qimbl : public QMainWindow {
  Q_OBJECT;

public:
  explicit Qimbl(QWidget *parent = 0);
  ~Qimbl();

private:
  Ui::Qimbl *ui;

  QEpicsPv * rfstat;
  QEpicsPv * rfcurrent;
  QEpicsPv * rfenergy;
  QEpicsPv * wigglergap;
  QEpicsPv * bl_enabled;
  QEpicsPv * bl_disabled;
  QEpicsPv * eps_enabled;
  QEpicsPv * eps_disabled;
  QEpicsPv * blmode1;
  QEpicsPv * blmode2;
  QEpicsPv * blmode3;

  QHash < Hutch*, QLabel* > hutches;

  ShutterFE * shfe;
  MrtShutter * shmrt;
  HhlSlitsGui * slits;
  FiltersGui * filters;
  MonoGui * mono;

  static const QStringList vacMonitors;
  static const QStringList tempMonitors;
  static const QStringList flowMonitors;
  QList<ValueBar*> vacBars;
  QList<ValueBar*> tempBars;
  QList<ValueBar*> flowBars;


private slots:

  void chooseComponent(QAbstractButton* but);

  void update_rfstat();
  void update_rfcurrent();
  void update_rfenergy();
  void update_wigglergap();
  void update_bl_status();
  void update_eps_status();
  void update_bl_mode();
  void update_hutches();
  void update_shfe();
  void update_shmrt();
  void update_slits();
  void update_filters();
  void update_mono();
  void update_vacuum();
  void update_temperature();
  void update_flow();


};




#endif // QIMBL_H
