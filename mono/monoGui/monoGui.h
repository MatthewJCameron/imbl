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
  void onStopReset();

  void updateConnection(bool con);
  void updateMotion(bool moving);
  void updateEnergyChanging();
  void updateInOut(Mono::InOutPosition iopos);
  void updateEnergy();

private:

};

#endif // MONOGUI_H

