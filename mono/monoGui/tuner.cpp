#include "tuner.h"
#include "ui_tuner.h"




Tuner::Tuner(QWidget *parent) :
  QDoubleSpinBox(parent),
  panel(new QWidget(0, Qt::Tool | Qt::FramelessWindowHint)),
  ui(new Ui::Tuner),
  meater(new WindowMotionEater(this))
{
  panel->setVisible(false);
  ui->setupUi(panel);

  ui->value->setRange(minimum(),maximum());
  ui->value->setDecimals(decimals());
  ui->value->setSuffix(suffix());
  ui->value->setPrefix(prefix());
  ui->value->setSingleStep(singleStep());
  ui->value->setSpecialValueText(specialValueText());
  ui->value->setValue(value());

  connect(ui->value, SIGNAL(valueEdited(double)), SIGNAL(valueChanged(double)));
  connect(ui->minus, SIGNAL(clicked()), SLOT(subtract()));
  connect(ui->plus, SIGNAL(clicked()), SLOT(add()));
  connect(ui->m2, SIGNAL(clicked()), SLOT(multiply2()));
  connect(ui->m10, SIGNAL(clicked()), SLOT(multiply10()));
  connect(ui->d2, SIGNAL(clicked()), SLOT(divide2()));
  connect(ui->d10, SIGNAL(clicked()), SLOT(divide10()));

  installEventFilter(this);
  ui->increment->installEventFilter(this);
  ui->value->installEventFilter(this);

  connect(meater, SIGNAL(windowMoved()), SLOT(updatePanel()));

}

Tuner::~Tuner() {
  delete ui;
}


void Tuner::updatePanel() {
  QPoint baseDSB = mapToGlobal(QPoint(0,0));
  QPoint showDSB = ui->value->mapToGlobal(QPoint(0,0));
  if (baseDSB != showDSB)
    panel->move(panel->pos() + baseDSB - showDSB);
}


bool Tuner::eventFilter(QObject *obj, QEvent *event) {
  if ( event->type() == QEvent::FocusIn ||
       event->type() == QEvent::FocusOut ) {
    panel->setVisible(
          hasFocus() ||
          ui->value->hasFocus() ||
          ui->increment->hasFocus() );
    if (hasFocus())
      ui->value->activateWindow();
  } else if ( obj == this && event->type() == QEvent::Resize ) {
    ui->value->setMinimumSize(width(), height());
    ui->value->setMaximumSize(width(), height());
  } else if ( obj == this && event->type() == QEvent::Move ) {
    updatePanel();
  } else if ( obj == this && event->type() == QEvent::ParentChange ) {
    meater->installMe();
  }

  return QObject::eventFilter(obj, event);
};


void Tuner::setValue(double val) {
  QDoubleSpinBox::setValue(val);
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

