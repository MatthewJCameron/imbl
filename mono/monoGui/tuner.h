#ifndef TUNER_H
#define TUNER_H

#include <QtGui/QWidget>
#include <QEvent>
#include<QDoubleSpinBox>
#include<QDebug>
#include<QTimer>

namespace Ui {
class Tuner;
}



class WindowMotionEater : public QObject {
  Q_OBJECT;
signals:
  void windowMoved();
public:
  inline WindowMotionEater(QWidget * parent) :
    QObject(parent)
  {
    QTimer::singleShot(0, this, SLOT(installMe()));
  }
protected:
  inline bool eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Move)
      emit windowMoved();
    return QObject::eventFilter(obj, event);
  }
public slots:
  inline void installMe() {
    QWidget * par = static_cast<QWidget*>(parent());
    if (par && par->nativeParentWidget())
      par->nativeParentWidget()->installEventFilter(this);
  }
};



class Tuner : public QDoubleSpinBox {
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

  void updatePanel();


private:
  QWidget * panel;
  Ui::Tuner *ui;
  WindowMotionEater * meater;

protected:
  bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TUNER_H


