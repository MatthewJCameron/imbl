#ifndef QIMBL_H
#define QIMBL_H

#include <QtGui/QMainWindow>
#include <QLabel>
#include "shutterFE.h"
#include "shutter1A.h"
#include "mrtShutter.h"
#include "hhlSlitsGui.h"
#include "filtersGui.h"
#include "monoGui.h"
#include "hutch.h"
#include "valve.h"
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

  QHash < Hutch*, QLabel* > hutches;

  ShutterFE * shfe;
  Shutter1A * sh1A;
  Valve * valve1;

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
  void update_sh1A();
  void update_shmrt();
  void update_slits();
  void update_filters();
  void update_mono();
  void update_vacuum();
  void update_temperature();
  void update_flow();
  void update_valve_1();


};


class QLabelLine : public QLabel {
public:
  inline explicit QLabelLine(QWidget * parent = 0, Qt::WindowFlags f = 0 ) :
    QLabel(parent,f) {}
  inline explicit QLabelLine (const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0 ) :
    QLabel(text,parent,f) {}
protected:
  virtual void paintEvent( QPaintEvent * event );
};

class QLabelLineMono : public QLabel {
public:
  inline explicit QLabelLineMono(QWidget * parent = 0, Qt::WindowFlags f = 0 ) :
    QLabel(parent,f) {}
  inline explicit QLabelLineMono (const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0 ) :
    QLabel(text,parent,f) {}
protected:
  virtual void paintEvent( QPaintEvent * event );
};


class QLabelLineTwo : public QLabel {
public:
  inline explicit QLabelLineTwo(QWidget * parent = 0, Qt::WindowFlags f = 0 ) :
    QLabel(parent,f) {}
  inline explicit QLabelLineTwo (const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0 ) :
    QLabel(text,parent,f) {}
protected:
  virtual void paintEvent( QPaintEvent * event );
};


class LinkedLeft : public QWidget {
public:
  inline explicit LinkedLeft (QWidget * parent = 0) : QWidget(parent) {};
protected:
  virtual void paintEvent( QPaintEvent * event );
};

class LinkedRight : public QWidget {
public:
  inline explicit LinkedRight (QWidget * parent = 0) : QWidget(parent) {};
protected:
  virtual void paintEvent( QPaintEvent * event );
};

class LinkedMiddle : public QWidget {
public:
  inline explicit LinkedMiddle (QWidget * parent = 0) : QWidget(parent) {};
protected:
  virtual void paintEvent( QPaintEvent * event );
};




#endif // QIMBL_H
