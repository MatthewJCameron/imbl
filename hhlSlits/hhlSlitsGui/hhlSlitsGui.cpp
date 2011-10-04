#include "hhlSlitsGui.h"
#include "ui_hhlSlitsGui.h"

#include <QPainter>
#include "shutterFE.h"
#include "error.h"







const float SlitsVis::slmargin = 0.01;
const float SlitsVis::emmargin = 0.01;


SlitsVis::SlitsVis(QWidget * parent)
  : QWidget(parent)
{
  setMinimumWidth(400);
  setMinimumHeight( minimumWidth() * HhlSlits::fullbeam.height() / HhlSlits::fullbeam.width() );
  QSizePolicy szpol(QSizePolicy::Ignored, QSizePolicy::Ignored);
  szpol.setHeightForWidth(true);
  setSizePolicy(szpol);
}



int
SlitsVis::heightForWidth ( int w ) const {
  return w * HhlSlits::fullbeam.height() / HhlSlits::fullbeam.width();
}



void
SlitsVis::update(double lValue, double rValue, double tValue, double bValue,
                double lGoal, double rGoal, double tGoal, double bGoal,
                bool) {
  double fw2 = HhlSlits::fullbeam.width()/2.0, fh2 = HhlSlits::fullbeam.height()/2.0;
  lG = qMin(fw2, lGoal);
  rG = qMin(fw2, rGoal);
  tG = qMin(fh2, tGoal);
  bG = qMin(fh2, bGoal);
  lV = qMin(fw2, lValue);
  rV = qMin(fw2, rValue);
  tV = qMin(fh2, tValue);
  bV = qMin(fh2, bValue);
  QWidget::update();
}

void
SlitsVis::paintEvent(QPaintEvent *) {

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  double scale =
  ( (double) height() / (double) width()  <
    HhlSlits::fullbeam.height() / HhlSlits::fullbeam.width() )  ?
  height() / HhlSlits::fullbeam.height()  :
  width() /  HhlSlits::fullbeam.width();

  painter.translate( width()/2.0 , height()/2.0);
  painter.scale( scale, -scale );

  double fw2 = HhlSlits::fullbeam.width()/2.0, fh2 = HhlSlits::fullbeam.height()/2.0;

  // beam
  painter.drawRect( QRectF(-fw2, -fh2,
                           HhlSlits::fullbeam.width(), HhlSlits::fullbeam.height() ));

  // coordinates
  painter.setPen( QPen( QBrush ( QColor(255,255,255)), 0, Qt::DotLine) );
  painter.drawLine(-fw2,0,fw2,0);
  painter.drawLine(0,-fh2,0,fh2);

  // current slits
  painter.setPen(QColor(255,0,0));
  painter.drawRect( QRectF( -lV, -bV, lV+rV, tV+bV ));

  // new slits
  painter.setPen(QColor(227,121,38,50));
  painter.setBrush(QColor(227,121,38,50));

  // bottom
  painter.drawRect( QRectF(-fw2, -fh2, HhlSlits::fullbeam.width(), fh2 - bG ) );
  // top
  painter.drawRect( QRectF(-fw2, fh2, HhlSlits::fullbeam.width(), tG - fh2 ) );
  // left
  painter.drawRect( QRectF(-fw2, -fh2, fw2 - lG, HhlSlits::fullbeam.height() ) );
  // right
  painter.drawRect( QRectF(fw2, -fh2, rG - fw2, HhlSlits::fullbeam.height() ) );

  // x-ray arrow
  //painter.setPen(QColor(255,0,0));
  //qreal rad=fh2/2.0;
  //painter.drawEllipse( QPointF(0,0), rad, rad);
  //painter.drawLine(-rad, rad, rad, -rad);
  //painter.drawLine(-rad, -rad, rad, rad);

}


















HhlSlitsGui:: HhlSlitsGui(HhlSlits * slt, QWidget *parent) :
  ComponentGui(slt, false, parent),
  vis(new SlitsVis(this)),
  ui(new Ui::HhlSlitsGui),
  motors(new QMotorStack(this))
{
  init();
}

HhlSlitsGui::HhlSlitsGui(QWidget *parent) :
  ComponentGui(new HhlSlits(parent), true, parent),
  vis(new SlitsVis(this)),
  ui(new Ui::HhlSlitsGui),
  motors(new QMotorStack(this))
{
  init();
}

HhlSlitsGui::~HhlSlitsGui() {
  delete motors;
  delete ui;
  delete vis;
}


void HhlSlitsGui::init() {

  ui->setupUi(this);
  ui->advanced->hide();
  ui->visLayout->addWidget(vis);

  motors->lock(true);
  motors->addMotor(HhlSlits::vPos, true, true);
  motors->addMotor(HhlSlits::vOpen, true, true);
  motors->addMotor(HhlSlits::hLeft, true, true);
  motors->addMotor(HhlSlits::hRight, true, true);

  ui->advancedLayout->addWidget(motors);

  connect(component(), SIGNAL(geometryChanged(double,double,double,double)), SLOT(updateGeometry()));
  connect(component(), SIGNAL(limitStateChanged(HhlSlits::Limits)), SLOT(updateLimits(HhlSlits::Limits)));
  connect(component(), SIGNAL(motionStateChanged(bool)), SLOT(updateMotion(bool)));

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->gostop_pb, SIGNAL(clicked()), SLOT(onGoPressed()));
  connect(ui->reset_pb, SIGNAL(clicked()), SLOT(onResetPressed()));
  connect(ui->calibrate_pb, SIGNAL(clicked()), SLOT(onAutoCalibration()));

  connect(ui->tpG, SIGNAL(valueChanged(double)), SLOT(onVtbGoalChanged()));
  connect(ui->bpG, SIGNAL(valueChanged(double)), SLOT(onVtbGoalChanged()));
  connect(ui->voG, SIGNAL(valueChanged(double)), SLOT(onVopGoalChanged()));
  connect(ui->vpG, SIGNAL(valueChanged(double)), SLOT(onVopGoalChanged()));

  connect(ui->lpG, SIGNAL(valueChanged(double)), SLOT(onHlrGoalChanged()));
  connect(ui->rpG, SIGNAL(valueChanged(double)), SLOT(onHlrGoalChanged()));
  connect(ui->hoG, SIGNAL(valueChanged(double)), SLOT(onHopGoalChanged()));
  connect(ui->hpG, SIGNAL(valueChanged(double)), SLOT(onHopGoalChanged()));

  updateGeometry();
  updateLimits(component()->limits());
  updateMotion(component()->isMoving());
  updateConnection(component()->isConnected());

}


void HhlSlitsGui::blockSetSignals(bool block) {
  foreach(QDoubleSpinBox *sb, ui->control->findChildren<QDoubleSpinBox *>())
    sb->blockSignals(block);
}



void HhlSlitsGui::updateGeometry() {
  ui->tpV->setText(QString::number(component()->top()));
  ui->bpV->setText(QString::number(component()->bottom()));
  ui->voV->setText(QString::number(component()->height()));
  ui->vpV->setText(QString::number(component()->vCenter()));
  ui->lpV->setText(QString::number(component()->left()));
  ui->rpV->setText(QString::number(component()->right()));
  ui->hoV->setText(QString::number(component()->width()));
  ui->hpV->setText(QString::number(component()->hCenter()));
  vis->update(component()->left(), component()->right(), component()->top(), component()->bottom(),
              ui->lpG->value(), ui->rpG->value(), ui->tpG->value(), ui->bpG->value());
}

void HhlSlitsGui::updateMotion(bool moving) {
  ui->control->setEnabled(!moving);
  ui->reset_pb->setEnabled(!moving);
  ui->calibrate_pb->setEnabled(!moving);
  ui->gostop_pb->setText(moving ? "STOP" : "Go");
  ui->reset_pb->setText("Reset");
}

void HhlSlitsGui::updateLimits(HhlSlits::Limits lms) {
  static const QString
      limitedStyle =
      "background-color: rgb(128, 0, 0);"
      " color: rgb(255, 255, 255);",
      unlimitedStyle = "";
  ui->voV->setStyleSheet( ( lms & HhlSlits::HeightHi || lms & HhlSlits::HeightLo ) ?
                            limitedStyle : unlimitedStyle );
  ui->vpV->setStyleSheet( ( lms & HhlSlits::ZPosHi || lms & HhlSlits::ZPosLo ) ?
                            limitedStyle : unlimitedStyle );
  ui->lpV->setStyleSheet( ( lms & HhlSlits::LeftHi || lms & HhlSlits::LeftLo ) ?
                            limitedStyle : unlimitedStyle );
  ui->rpV->setStyleSheet( ( lms & HhlSlits::RightHi || lms & HhlSlits::RightLo ) ?
                            limitedStyle : unlimitedStyle );
}

void HhlSlitsGui::updateConnection(bool con) {
  foreach(QWidget* widg, findChildren<QWidget *>() )
    if (widg != ui->advanced_pb && widg != ui->advanced)
      widg->setEnabled(con);
  if (con) {
    QTimer::singleShot(0, this, SLOT(onResetPressed()));
    updateMotion(component()->isMoving());
  } else {
    ui->gostop_pb->setText("Disconnected");
    ui->reset_pb->setText("Disconnected");
    ui->lpV->setText("Disconnected");
    ui->rpV->setText("Disconnected");
    ui->hpV->setText("Disconnected");
    ui->hoV->setText("Disconnected");
    ui->bpV->setText("Disconnected");
    ui->tpV->setText("Disconnected");
    ui->vpV->setText("Disconnected");
    ui->voV->setText("Disconnected");
  }
}

void HhlSlitsGui::onHopGoalChanged() {
  blockSetSignals(true);
  QPair<double,double> lr = HhlSlits::oc2pn(ui->hoG->value(), ui->hpG->value());
  ui->lpG->setValue(lr.first);
  ui->rpG->setValue(lr.second);
  blockSetSignals(false);
  updateGeometry();
}

void HhlSlitsGui::onHlrGoalChanged() {
  blockSetSignals(true);
  QPair<double,double> wy = HhlSlits::pn2oc(ui->lpG->value(), ui->rpG->value());
  ui->hoG->setValue(wy.first);
  ui->hpG->setValue(wy.second);
  blockSetSignals(false);
  updateGeometry();
}

void HhlSlitsGui::onVopGoalChanged() {
  blockSetSignals(true);
  QPair<double,double> tb = HhlSlits::oc2pn(ui->voG->value(), ui->vpG->value());
  ui->tpG->setValue(tb.first);
  ui->bpG->setValue(tb.second);
  blockSetSignals(false);
  updateGeometry();
}

void HhlSlitsGui::onVtbGoalChanged() {
  blockSetSignals(true);
  QPair<double,double> hz = HhlSlits::pn2oc(ui->lpG->value(), ui->rpG->value());
  ui->voG->setValue(hz.first);
  ui->vpG->setValue(hz.second);
  blockSetSignals(false);
  updateGeometry();
}

void HhlSlitsGui::onGoPressed() {
  if ( component()->isMoving() )
    component()->stop(false);
  else
    component()->setGeometry(ui->lpG->value(), ui->rpG->value(),
                             ui->voG->value(), ui->vpG->value(), false);
}

void HhlSlitsGui::onResetPressed() {
  blockSetSignals(true);
  ui->voG->setValue(component()->height());
  ui->vpG->setValue(component()->vCenter());
  ui->tpG->setValue(component()->top());
  ui->bpG->setValue(component()->bottom());
  ui->hpG->setValue(component()->hCenter());
  ui->hoG->setValue(component()->width());
  ui->lpG->setValue(component()->left());
  ui->rpG->setValue(component()->right());
  blockSetSignals(false);
  updateGeometry();
}

void HhlSlitsGui::onAdvancedControl() {
  if (ui->advanced->isVisible()) {
    ui->advanced->hide();
    ui->advanced_pb->setText("Show advanced control");
    ui->advanced_pb->setStyleSheet("");
  } else if ( PsswDial::ask(this) ) {
    ui->advanced->show();
    ui->advanced_pb->setText("Please don't forget to close");
    ui->advanced_pb->setStyleSheet("background-color: rgba(255, 0, 0,64);");
  }
}

void HhlSlitsGui::onAutoCalibration() {

  if ( ! PsswDial::ask(this) )
    return;

  ShutterFE::State inst(ShutterFE::stateS());
  if ( ! ShutterFE::setOpenedS(false,true) ) {
    warn("Can't close the FE shutter."
         " Calibration failed. Try to repeat or do it manually.", this);
    return;
  }

  // get current positions
  double invo = HhlSlits::vOpen->get();
  double invp = HhlSlits::vPos->get();
  double inhl = HhlSlits::hLeft->get();
  double inhr = HhlSlits::hRight->get();

  // set software limits to something what is guaranteed to be beyond the real limit
  HhlSlits::vOpen->setUserLoLimit( -qAbs(invo) - 2 * HhlSlits::fullbeam.height() );
  HhlSlits::vPos->setUserLoLimit( -qAbs(invp) - 2 * HhlSlits::fullbeam.height() );
  HhlSlits::hLeft->setUserLoLimit( -qAbs(inhl) - HhlSlits::fullbeam.width() );
  HhlSlits::hRight->setUserLoLimit( -qAbs(inhr) - HhlSlits::fullbeam.width() );

  // go to find the hardware limits
  HhlSlits::vOpen->goLimit(-1);
  HhlSlits::vPos->goLimit(-1);
  HhlSlits::hLeft->goLimit(-1);
  HhlSlits::hRight->goLimit(-1);

  component()->wait_stop();

  if ( ! HhlSlits::vOpen->getLoLimitStatus() ||
       ! HhlSlits::vPos->getLoLimitStatus() ||
       ! HhlSlits::hLeft->getLoLimitStatus() ||
       ! HhlSlits::hRight->getLoLimitStatus() ) {
    warn("Could not reach at least one of the motor limits."
         " Calibration failed. Try to repeat or do it manually.", this);
    return;
  }

  // calibratre
  HhlSlits::vOpen->setUserPosition(HhlSlits::vOpenCalibre);
  HhlSlits::vPos->setUserPosition(HhlSlits::vPosCalibre);
  HhlSlits::hLeft->setUserPosition(HhlSlits::hLeftCalibre);
  HhlSlits::hRight->setUserPosition(HhlSlits::hRightCalibre);

  // restore software limits
  HhlSlits::vOpen->setUserLoLimit(0);
  HhlSlits::vPos->setUserLoLimit(HhlSlits::vPosCalibre);
  HhlSlits::hLeft->setUserLoLimit(0);
  HhlSlits::hRight->setUserLoLimit(0);

  // return to the initial positions
  HhlSlits::vOpen->goUserPosition(invo);
  HhlSlits::vPos->goUserPosition(invp);
  HhlSlits::hLeft->goUserPosition(inhl);
  HhlSlits::hRight->goUserPosition(inhr);

  ShutterFE::setOpenedS(inst == ShutterFE::OPENED);

}

