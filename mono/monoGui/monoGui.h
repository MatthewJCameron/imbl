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

  void updateConnection(bool con);
  void updateMotion(bool moving);
  void updateInOut(Mono::InOutPosition iopos);
  void updateEnergy();
  void updateDBragg();
  void updateDX();
  void updateZseparation();
  void updateTilt1();
  void updateTilt2();
  void updateBend1f();
  void updateBend1b();
  void updateBend2f();
  void updateBend2b();
  void updateMotorBragg1();
  void updateMotorBragg2();
  void updateMotorX();


private:
  EnergySetRevert * energySetter;

};

#endif // MONOGUI_H

