#ifndef HHLSLITSGUI_H
#define HHLSLITSGUI_H

#include <qmotorstack.h>
#include "componentGui.h"
#include "hhlSlits.h"

namespace Ui {
class HhlSlitsGui;
}


class SlitsVis : public QWidget {
  Q_OBJECT;
private:
  static const float slmargin;
  static const float emmargin;
  double lG;
  double rG;
  double tG;
  double bG;
  double lV;
  double rV;
  double tV;
  double bV;
  bool mirr;

public:

  SlitsVis(QWidget * parent);

  void update(double lValue, double rValue, double tValue, double bValue,
              double lGoal, double rGoal, double tGoal, double bGoal,
              bool = false);
  int heightForWidth ( int w ) const;
  void paintEvent(QPaintEvent *);

};


class HhlSlitsGui : public ComponentGui {
  Q_OBJECT;

  explicit HhlSlitsGui(HhlSlits * slt, QWidget *parent = 0);
  void init();

  void blockSetSignals(bool block);

public:

  explicit HhlSlitsGui(QWidget *parent = 0);
  ~HhlSlitsGui();

  inline HhlSlitsGui * duplicate(QWidget * parent=0) { return new HhlSlitsGui(component(), parent); }

  inline HhlSlits * component() const {return static_cast<HhlSlits*>( ComponentGui::component() );}

private:

  SlitsVis * vis;
  Ui::HhlSlitsGui *ui;

private slots:

  void updateGeometry();
  void updateMotion(bool moving);
  void updateLimits(HhlSlits::Limits lms);
  void updateConnection(bool con);

  void onHopGoalChanged();
  void onHlrGoalChanged();
  void onVopGoalChanged();
  void onVtbGoalChanged();

  void onGoPressed();
  void onResetPressed();
  void onAdvancedControl();
  void onAutoCalibration();

};


class QDoubleSpinBoxEntered : public QDoubleSpinBox {
  Q_OBJECT;
public:
  inline QDoubleSpinBoxEntered(QWidget * parent =0) : QDoubleSpinBox(parent) {  }
signals :
    void enterPressed();
protected:
  inline void keyPressEvent( QKeyEvent * event ) {
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
      emit enterPressed();
    QDoubleSpinBox::keyPressEvent(event);
  }
};



#endif // HHLSLITSGUI_H
