#ifndef TUNER_H
#define TUNER_H

#include <QtGui/QWidget>
#include <QEvent>

namespace Ui {
class Tuner;
}

class Tuner : public QWidget {
  Q_OBJECT;

public:
  explicit Tuner(QWidget *parent = 0);
  ~Tuner();

public slots:
  void setValue(double val);

signals:
  void valueChanged(double);

private slots:

  void add();
  void subtract();
  void multiply2();
  void multiply10();
  void divide2();
  void divide10();

private:
  Ui::Tuner *ui;

protected:
  bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TUNER_H
