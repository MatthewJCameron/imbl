#ifndef expanderGUI_H
#define expanderGUI_H

#include <QWidget>
#include <qmotorstack.h>
#include "componentGui.h"
#include "expander.h"

namespace Ui {
class ExpanderGui;
class EnergySetter;
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
  Ui::EnergySetter * ui;
  bool eventFilter(QObject *obj, QEvent *event);
};

class ExpanderGui : public ComponentGui {
  Q_OBJECT;

private:

  Ui::ExpanderGui *ui;

  explicit ExpanderGui(Expander * expander, QWidget *parent = 0);
  
  void init();

public:

  explicit ExpanderGui(QWidget *parent = 0);
  ~ExpanderGui();

  inline ExpanderGui * duplicate(QWidget * parent=0) { return new ExpanderGui(component(), parent); }
  inline Expander * component() const {return static_cast<Expander*>( ComponentGui::component() );}

private slots:

  void onAdvancedControl();
  //void updateStatus();
  void updateConnection(bool con);
  void updateExpInOut(Expander::InOutPosition iopos);
  void updateTblInOut(Expander::InOutPosition iopos);
  void onEnergySet();
  void onEnergyTune();
  void updateEnergy();
  void revertEnergy();
  void updateDBragg();
  void updateEnergyMotion();

private:
  EnergySetRevert * energySetter;
  bool eventFilter(QObject *obj, QEvent *event);

};

class EnterEscapePressEater : public QObject {
  Q_OBJECT;
public:
  inline explicit EnterEscapePressEater (QWidget * parent = 0) : QObject(parent) {};
protected:
  virtual bool eventFilter(QObject * obj, QEvent * event);
signals:
  void enterPressed();
  void escapePressed();
};


#endif // expanderGUI_H

