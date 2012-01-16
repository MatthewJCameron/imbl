#ifndef FILTERSGUI_H
#define FILTERSGUI_H

#include "componentGui.h"
#include "filters.h"
#include <qmotorstack.h>
#include <qwt_plot_curve.h>

namespace Ui {
    class Filters;
    class Paddle;
}




class PaddleGui : public ComponentGui {
  Q_OBJECT;

private:

  Ui::Paddle *ui;

public:

  PaddleGui(Paddle* _pad, QWidget* parent=0);
  ~PaddleGui();

  int selectedWindow() const ;
  inline Absorber selectedAbsorber() const { return component()->absorber(selectedWindow()); }
  inline PaddleGui * duplicate(QWidget * parent=0) { return new PaddleGui(component(), parent); }
  inline Paddle * component() const {return static_cast<Paddle*>( ComponentGui::component() );}

public slots:

  void selectWindow(int win);
  inline void reset() { selectWindow(component()->window()); }

signals:
  void selectedChanged(int idx);

private slots:

  void updateConnection(bool con);
  void updateWindow(int win);
  void updateMotionState(bool mov);
  void updateLabel();

};





class FiltersGui : public ComponentGui {
  Q_OBJECT;

private:

  Ui::Filters *ui;
  QList<PaddleGui*> paddles;

  QwtPlotCurve * wb_curve;
  QwtPlotCurve * current_curve;
  QwtPlotCurve * new_curve;

  static const bool data_inited;

  explicit FiltersGui(Filters * flt, QWidget *parent = 0);
  void init();

  QList<int> selectedWindows;

public:
  explicit FiltersGui(QWidget *parent = 0);
  ~FiltersGui();

  inline FiltersGui * duplicate(QWidget * parent=0) { return new FiltersGui(component(), parent); }
  inline Filters * component() const {return static_cast<Filters*>( ComponentGui::component() );}

private slots:

  void updateConnection(bool con);
  void updateTrain();
  void updateSelection();
  void updateMotion(bool mov);

  void onGoPressed();
  void onResetPressed();
  void onAdvancedControl();
  void onAutoCalibration();

};


#endif // FILTERSGUI_H
