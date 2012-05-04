#include "valuebar.h"
#include "ui_valuebar.h"
#include "error.h"

#include <QPainter>
#include <QDebug>
#include <math.h>

#include <qwt_scale_map.h>
#include <qwt_scale_engine.h>


void ExtendedThermo::paintEvent( QPaintEvent * event ){

  QPainter painter(this);
  QPen pen;
  pen.setCapStyle(Qt::RoundCap);
  pen.setWidth(3);
  const int top = rect().top(), bot=rect().bottom();

  int barVal;

  pen.setColor(Qt::yellow);
  painter.setPen(pen);
  if ( ! isnan(lo()) && lo() >= minValue() ) {
    barVal= qRound( scaleMap().transform(lo()) );
    painter.drawLine(barVal, bot, barVal, top);
  }
  if ( ! isnan(hi()) && hi() <= maxValue() ) {
    barVal= qRound( scaleMap().transform(hi()) );
    painter.drawLine(barVal, bot, barVal, top);
  }
  pen.setColor(Qt::red);
  painter.setPen(pen);
  if ( ! isnan(lolo()) && lolo() >= minValue() ) {
    barVal= qRound( scaleMap().transform(lolo()) );
    painter.drawLine(barVal, bot, barVal, top);
  }
  if ( ! isnan(hihi()) && hihi() <= maxValue() ) {
    barVal= qRound( scaleMap().transform(hihi()) );
    painter.drawLine(barVal, bot, barVal, top);
  }

  QwtThermo::paintEvent(event);

}









ValueBar::ValueBar(const QString &_pv, QWidget *parent):
  QWidget(parent),
  ui(new Ui::ValueBar),
  pv(new QEpicsPv(_pv,this)),
  lolo(NAN),
  lo(NAN),
  hi(NAN),
  hihi(NAN),
  min(NAN),
  max(NAN),
  prec(0)
{
  ui->setupUi(this);
  ui->title->setAlignment( Qt::AlignCenter );
  connect(pv, SIGNAL(valueUpdated(QVariant)), SLOT(updateValue()));
  connect(pv, SIGNAL(connectionChanged(bool)), SLOT(updateConnection()));
}


ValueBar::~ValueBar() {
  delete ui;
  delete pv;
}


QGridLayout * ValueBar::internalLayout() {
  return ui->allignLayout;
}


void ValueBar::setLogarithmic(bool lg) {
  if ( lg ) ui->thermo->setScaleEngine( new QwtLog10ScaleEngine  );
  else      ui->thermo->setScaleEngine( new QwtLinearScaleEngine );
}


bool ValueBar::isLogarithmic() {
  return
      ui->thermo->scaleEngine()->transformation()->type() ==
      QwtScaleTransformation::Log10;
}


void ValueBar::updateValue() {

  if ( ! pv->isConnected() )
    return;

  bool ok;
  double value = pv->get().toDouble(&ok);
  ui->thermo->setValue(value);
  if ( isLogarithmic() || qAbs(value) > 1.0e05 || ( qAbs(value) < 1.0e-05 && value != 0.0 ) )
    ui->value->setText( QString::number(value,'e',prec)+units );
  else
    ui->value->setText( QString::number(value,'f',prec)+units );
  ui->value->setStyleSheet("");

  if (!ok) {
    ui->thermo->setFillBrush(Qt::red);
    ui->thermo->setValue(ui->thermo->maxValue());
    ui->value->setText("Error");
    ui->value->setStyleSheet("color: rgb(255, 0, 0);");
    setHealth(ALARM);
  } else if ( ( ! isnan(ui->thermo->lolo()) && value <= ui->thermo->lolo() ) ||
              ( ! isnan(ui->thermo->hihi()) &&value >= ui->thermo->hihi() ) ) {
    ui->thermo->setFillBrush(Qt::red);
    ui->value->setStyleSheet("color: rgb(255, 0, 0);");
    setHealth(ALARM);
  } else if ( ( ! isnan(ui->thermo->lo()) && value <= ui->thermo->lo() ) ||
              ( ! isnan(ui->thermo->hi()) && value >= ui->thermo->hi() ) ) {
    ui->thermo->setFillBrush(Qt::yellow);
    ui->value->setStyleSheet("color: rgb(255, 128, 0);");
    setHealth(WARN);
  } else {
    ui->thermo->setFillBrush(Qt::green);
    ui->value->setStyleSheet("");
    setHealth(OK);
  }

}


void ValueBar::updateConnection() {

  if ( ! pv->isConnected() ) {
    ui->thermo->setFillBrush(Qt::red);
    ui->thermo->setValue(ui->thermo->maxValue());
    ui->title->setText(pv->pv());
    ui->title->setStyleSheet("color: rgb(255, 0, 0);");
    ui->value->setText("No link");
    ui->value->setStyleSheet("color: rgb(255, 0, 0);");
    setHealth(ALARM);
  } else {
    ui->title->setStyleSheet("");
    updateParams();
  }

}


void ValueBar::updateParams() {

  if (!pv->isConnected())
    return;

  QVariant qv;

  if ( ! isnan(lolo) )
    ui->thermo->setLL(lolo);
  else {
    qv=QEpicsPv::get(pv->pv()+".LOLO");
    if ( qv.isValid() && qv.canConvert(QVariant::Double) )
      ui->thermo->setLL(qv.toDouble());
    else
      ui->thermo->setLL();
  }

  if ( ! isnan(lo) )
    ui->thermo->setL(lo);
  else {
    qv=QEpicsPv::get(pv->pv()+".LOW");
    if ( qv.isValid() && qv.canConvert(QVariant::Double) )
      ui->thermo->setL(qv.toDouble());
    else
      ui->thermo->setL();
  }

  if ( ! isnan(hi) )
    ui->thermo->setH(hi);
  else {
    qv=QEpicsPv::get(pv->pv()+".HIGH");
    if ( qv.isValid() && qv.canConvert(QVariant::Double) )
      ui->thermo->setH(qv.toDouble());
    else
      ui->thermo->setH();
  }

  if ( ! isnan(hihi) )
    ui->thermo->setHH(hihi);
  else {
    qv=QEpicsPv::get(pv->pv()+".HIHI");
    if ( qv.isValid() && qv.canConvert(QVariant::Double) )
      ui->thermo->setHH(qv.toDouble());
    else
      ui->thermo->setHH();
  }

  if ( ! isnan(min) )
    ui->thermo->setMinValue(min);
  if ( ! isnan(max) )
    ui->thermo->setMaxValue(max);
  if ( isnan(min) || isnan(max) ) {
    double aMin = ( isnan(min) ? ui->thermo->lolo() : min );
    double aMax = ( isnan(max) ? ui->thermo->hihi() : max );
    if ( isnan(aMin) || isnan(aMax) ) {
      warn("Limit(s) of the value bar not set manually"
           " and could not be determined from the PV's HIHI and LOLO fields.");
    } else {
      double delta;
      if ( isLogarithmic() )
        delta = (aMax>0 && aMin>0) ? pow( aMax/aMin, 0.05) : 1.0;
      else
        delta = (aMax - aMin) * 0.05;
      if (isnan(min)) {
        if ( isLogarithmic() )
          ui->thermo->setMinValue(ui->thermo->lolo() / delta);
        else
          ui->thermo->setMinValue(ui->thermo->lolo() - delta);
      }
      if (isnan(max)) {
        if ( isLogarithmic() )
          ui->thermo->setMaxValue(ui->thermo->hihi() * delta);
        else
          ui->thermo->setMaxValue(ui->thermo->hihi() + delta);
      }
    }
  }

  qv=QEpicsPv::get(pv->pv()+".DESC");
  if ( qv.isValid() )
    ui->title->setText(qv.toString());

  qv=QEpicsPv::get(pv->pv()+".EGU");
  if ( qv.isValid() )
    units = qv.toString();

  qv=QEpicsPv::get(pv->pv()+".PREC");
  if ( qv.isValid() && qv.canConvert(QVariant::Int))
    prec = qv.toInt();

  updateValue();

}
