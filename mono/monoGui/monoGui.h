#ifndef MONOGUI_H
#define MONOGUI_H

#include <QWidget>
#include <qmotorstack.h>
#include "componentGui.h"
#include "mono.h"

namespace Ui {
class MonoGui;
}

class Tuner;



class EnergySetRevert : public QWidget {
  Q_OBJECT;
public:
  EnergySetRevert(QWidget * master);
signals:
  void revert();
  void set();
private:
  QPushButton * setBut;
  QPushButton * revertBut;
  bool eventFilter(QObject *obj, QEvent *event);
};




class MonoGui : public ComponentGui {
  Q_OBJECT;

private:

  Ui::MonoGui *ui;

  explicit MonoGui(Mono * mono, QWidget *parent = 0);
  bool dBraggHealth;
  bool dXhealth;
  bool dZhealth;


  void init();

public:

  explicit MonoGui(QWidget *parent = 0);
  ~MonoGui();

  inline MonoGui * duplicate(QWidget * parent=0) { return new MonoGui(component(), parent); }
  inline Mono * component() const {return static_cast<Mono*>( ComponentGui::component() );}

private slots:

  void onAdvancedControl();
  void onEnergySet();
  void onEnergyTune();
  void onZseparationSet();

  void updateConnection(bool con);
  void updateInOut(Mono::InOutPosition iopos);
  void updateEnergy();
  void updateDBragg();
  void updateDX();
  void updateDZ();
  void updateTilt1();
  void updateTilt2();
  void updateBend1ob();
  void updateBend1ib();
  void updateBend2ob();
  void updateBend2ib();
  void updateMotorBragg1();
  void updateMotorBragg2();
  void updateMotorX();
  void updateStatus();
  void updateEnergyMotion();

  void updateLSs();


private:
  EnergySetRevert * energySetter;
  bool eventFilter(QObject *obj, QEvent *event);

};



#endif // MONOGUI_H

