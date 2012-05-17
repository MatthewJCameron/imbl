#ifndef TUNER_H
#define TUNER_H

#include <QWidget>
#include <qtpvwidgets.h>
#include <QEvent>

namespace Ui {
class Tuner;
}

class TunerIncrement : public QDoubleSpinBox {
  Q_OBJECT;
public:
  inline explicit TunerIncrement(QWidget *parent = 0) : QDoubleSpinBox(parent) {}
public slots:
  inline void onMotionChange(bool mov) {lineEdit()->setEnabled(!mov); setReadOnly(mov);}
};


class Tuner : public QMDoubleSpinBox {
  Q_OBJECT;

public:
  explicit Tuner(QWidget *parent = 0);
  ~Tuner();
  void setIncrement(double twk);
  void setDecimals(int prec);

public slots:
  void onMotionChange(bool mov);

private slots:

  void add();
  void subtract();
  void multiply2();
  void multiply10();
  void divide2();
  void divide10();

private:

  QWidget * control;
  Ui::Tuner *ui;

protected:
  bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TUNER_H
