#ifndef TUNER_H
#define TUNER_H

#include <QWidget>
#include <qtpvwidgets.h>
#include <QEvent>

namespace Ui {
class Tuner;
}

class Tuner : public QMDoubleSpinBox {
  Q_OBJECT;

public:
  explicit Tuner(QWidget *parent = 0);
  ~Tuner();

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
