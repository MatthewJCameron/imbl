#ifndef QIMBL_H
#define QIMBL_H

#include <QtGui/QMainWindow>
#include "shutterFE.h"
#include "mrtShutter.h"
#include "hhlSlits.h"
#include "filters.h"
#include "mono.h"
#include "hutch.h"
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

  Hutch * h1A;
  Hutch * h1B;
  Hutch * h2A;
  Hutch * h2B;
  Hutch * tunnel;
  Hutch * h3A;
  Hutch * h3B;

  ShutterFE * shfe;
  MrtShutter * shmrt;
  HhlSlits * slits;
  Filters * filters;
  Mono * mono;




private slots:

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

};

#endif // QIMBL_H
