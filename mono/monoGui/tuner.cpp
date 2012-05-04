#include "tuner.h"
#include "ui_tuner.h"

Tuner::Tuner(QWidget *parent) :
  QMDoubleSpinBox(parent),
  control(new QWidget(parent)),
  ui(new Ui::Tuner)
{
  ui->setupUi(control);
  connect(ui->minus, SIGNAL(clicked()), SLOT(subtract()));
  connect(ui->plus, SIGNAL(clicked()), SLOT(add()));
  connect(ui->m2, SIGNAL(clicked()), SLOT(multiply2()));
  connect(ui->m10, SIGNAL(clicked()), SLOT(multiply10()));
  connect(ui->d2, SIGNAL(clicked()), SLOT(divide2()));
  connect(ui->d10, SIGNAL(clicked()), SLOT(divide10()));
  installEventFilter(this);
  lineEdit()->installEventFilter(this);
  ui->increment->installEventFilter(this);
  control->setVisible(false);
  control->setAutoFillBackground(true);
}

Tuner::~Tuner() {
  delete ui;
}


bool Tuner::eventFilter(QObject *obj, QEvent *event) {
  if ( (obj == this || obj == ui->increment) &&
       (event->type() == QEvent::FocusIn ||
        event->type() == QEvent::FocusOut ) ) {
    control->setVisible( hasFocus() ||
                         ui->increment->hasFocus() );
    control->raise();
  } else if ( obj== lineEdit() && event->type() == QEvent::MouseButtonPress ) {
    control->setVisible(!control->isVisible());
  } else if ( obj==this && event->type() == QEvent::Resize ) {
    control->setMinimumWidth(width());
    control->setMaximumWidth(width());
  } else if ( obj==this && ( event->type() == QEvent::Move ||
                             event->type() == QEvent::Show ) ) {
    QRect geom = control->geometry();
    geom.moveTopLeft(geometry().topLeft()+QPoint(0, height()));
    control->setGeometry(geom);
  }
  return QObject::eventFilter(obj, event);
}


void Tuner::add() {
  double newVal = value() + ui->increment->value();
  setValue( newVal );
  QDoubleSpinBox::setValue( newVal );
  emit valueEdited(newVal);
}

void Tuner::subtract() {
  double newVal = value() - ui->increment->value();
  setValue( newVal );
  QDoubleSpinBox::setValue( newVal );
  emit valueEdited(newVal);
}

void Tuner::multiply2() {
  ui->increment->setValue(2*ui->increment->value());
}

void Tuner::multiply10() {
  ui->increment->setValue(10*ui->increment->value());
}

void Tuner::divide2() {
  ui->increment->setValue(0.5*ui->increment->value());
}

void Tuner::divide10() {
  ui->increment->setValue(0.1*ui->increment->value());
}

void Tuner::setIncrement(double twk) {
  ui->increment->setValue(twk);
}

void Tuner::setDecimals(int prec) {
  QDoubleSpinBox::setDecimals(prec);
  ui->increment->setDecimals(prec);
}

