#include "tuner.h"
#include "ui_tuner.h"

Tuner::Tuner(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Tuner)
{
  ui->setupUi(this);
  connect(ui->value, SIGNAL(valueEdited(double)), SIGNAL(valueChanged(double)));
  connect(ui->minus, SIGNAL(clicked()), SLOT(subtract()));
  connect(ui->plus, SIGNAL(clicked()), SLOT(add()));
  connect(ui->m2, SIGNAL(clicked()), SLOT(multiply2()));
  connect(ui->m10, SIGNAL(clicked()), SLOT(multiply10()));
  connect(ui->d2, SIGNAL(clicked()), SLOT(divide2()));
  connect(ui->d10, SIGNAL(clicked()), SLOT(divide10()));
  ui->increment->installEventFilter(this);
  ui->value->installEventFilter(this);
  ui->control->setVisible(false);
}

Tuner::~Tuner() {
  delete ui;
}


bool Tuner::eventFilter(QObject *obj, QEvent *event) {
  if ( event->type() == QEvent::FocusIn ||
       event->type() == QEvent::FocusOut )
    ui->control->setVisible(ui->value->hasFocus() || ui->increment->hasFocus());
  return QObject::eventFilter(obj, event);
}


void Tuner::setValue(double val) {
  ui->value->setValue(val);
}

void Tuner::add() {
  ui->value->setValue( ui->value->value() + ui->increment->value() );
  emit valueChanged(ui->value->value());
}

void Tuner::subtract() {
  ui->value->setValue( ui->value->value() - ui->increment->value() );
  emit valueChanged(ui->value->value());
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

