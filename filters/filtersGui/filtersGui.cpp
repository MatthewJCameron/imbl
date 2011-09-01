#include "filtersGui.h"
#include "ui_filtersGui.h"
#include "ui_paddle.h"
#include "error.h"
#include "shutterFE.h"
#include <QtSvg/QtSvg>

#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_grid.h>

PaddleGui::PaddleGui(Paddle *_pad, QWidget *parent) :
  ComponentGui(_pad,false,parent),
  ui(new Ui::Paddle)
{

  ui->setupUi(this);

  connect(component(), SIGNAL(motionStateChanged(bool)), SLOT(updateMotionState(bool)));
  connect(component(), SIGNAL(windowChanged(int)), SLOT(updateWindow(int)));
  connect(component(), SIGNAL(limitStateChanged(bool)), SLOT(updateLabel()));

  ui->buttonGroup = new QButtonGroup(this);
  const QList<Paddle::Window> & windows = component()->windows();
  for(int idx = 0 ; idx<windows.size() ; idx++ ) {
    QRadioButton * curbut = new QRadioButton(windows[idx].second.description(), this);
    curbut->setToolTip(QString::number(windows[idx].first));
    ui->buttonGroup->addButton(curbut,idx);
    ui->verticalLayout->addWidget(curbut);
  }

  connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), SIGNAL(selectedChanged(int)));

  adjustSize();
  setMinimumSize(frameSize());
  updateConnection(component()->isConnected());

}

PaddleGui::~PaddleGui() {
  delete ui;
}

int PaddleGui::selectedWindow() const {
  return ui->buttonGroup->checkedId();
}

void PaddleGui::selectWindow(int win) {
  /*
  if ( win < 0 || win >= ui->buttonGroup->buttons().size() )
    warn("Request to select non-existing window in the paddle.", this);
  else
    ui->buttonGroup->buttons()[win]->setChecked(true);
    */
  if ( win >= 0 && win < ui->buttonGroup->buttons().size() )
    ui->buttonGroup->buttons()[win]->setChecked(true);
}

void PaddleGui::updateConnection(bool con) {
  setEnabled(con);
  updateLabel();
  if ( component()->isConnected() ) {
    updateWindow(component()->window());
    updateMotionState(component()->isMoving());
  }
}

void PaddleGui::updateWindow(int win) {
  foreach(QAbstractButton* but, ui->buttonGroup->buttons() )
    but->setStyleSheet("");
  if ( win >= 0 )
    ui->buttonGroup->button(win)->setStyleSheet("background-color: rgb(170, 85, 0);");
  updateLabel();
}

void PaddleGui::updateMotionState(bool mov) {
  foreach(QAbstractButton* but, ui->buttonGroup->buttons() )
    but->setEnabled(!mov);
  updateLabel();
}


void PaddleGui::updateLabel() {
  QString text;
  if ( ! component()->isConnected() )
    text = "Disconnected";
  else if ( component()->isMoving() )
    text = QString::number(component()->motor()->getUserPosition());
  else if (component()->isOnLimit())
    text = "On limit";
  else if (component()->window() < 0)
    text = "Mispositioned!";
  else
    text = QString::number( component()->inclination() * 180 / M_PI ) + QString::fromUtf8("°");
  ui->label->setText(text);
}








static double max(const QVector<double> & vec){
  if ( vec.isEmpty() )
    warn("Operation on empty vector.");
  double cmax = vec[0];
  foreach (double cur, vec)
    if (cur > cmax) cmax = cur;
  return cmax;
}

static const QVector<double> operator*(const QVector<double> & vec, double cof) {
  QVector<double> nvec;
  for (int i=0 ; i<vec.size() ; i++)
    nvec << vec[i] * cof;
  return nvec;
}

static void loadData ( const QString & filename, QPolygonF & storage) {
  storage.clear();
  QFile funcf(filename);
  if ( ! funcf.open(QIODevice::ReadOnly) )
    throw_error("Could not open input file \"" + filename + "\".");
  QString rd;
  while ( ! funcf.atEnd()  &&   !(rd = funcf.readLine()).isEmpty() ) {
    float toread1, toread2;
    if ( sscanf( rd.toAscii(), "%f %f\n", &toread1, &toread2 ) != 2 )
      throw_error("Could not scan float from input file"
                  " \"" + filename + "\".");
    storage << QPointF(toread1,toread2);
  }
  funcf.close();
}


static void loadMu(Absorber::Material mat, QVector<double> & dat, const QVector<double> & en){
  int size = en.size();
  QPolygonF data;
  loadData(":/mu-" + materialName(mat) + ".dat", data);
  if (data.size() != size)
    throw_error("File with the " + materialName(mat)
                + " absorption coefficient has unexpected size ("
                + QString::number(data.size()) + " != " + QString::number(size) + ").");
  for (int i = 0; i<size ; i++) {
    if ( en[i] != data[i].x() )
      throw_error("X axis of the " + materialName(mat)
                  + " absorption coefficient data do not match one of the whitebeam."
                  + " See string " + QString::number(i) + ": "
                  + QString::number(en[i]) + " != " + QString::number(data[i].x()) + "."  );
    else
      dat << data[i].y();
  }
}





const double spectrumMin=1.0e-11;
static QVector<double> energies;
static QVector<double> whitebeam;
static QHash< Absorber::Material, QVector<double> > mu;

void absorb(QVector<double> & inout, const QList<Absorber::Foil> & train) {
  const int size = whitebeam.size();
  inout = whitebeam * ( 1/max(whitebeam) );
  foreach(Absorber::Foil foil, train)
    if ( foil.first != Absorber::Empty && foil.second > 0.0 )
      for (int i=0 ; i<size ; i++)
        inout[i] *= exp( - mu[foil.first][i] * foil.second * 0.1);
  for (int i=0 ; i<size ; i++)
    if ( inout[i] < spectrumMin/10.0 )
      inout[i] = spectrumMin/10.0;
}


static bool init_data() {

  QPolygonF data;
  loadData(":/flux.dat", data);
  if (data.size()<=100)
    throw_error("File with the white beam spectrum has size less than 100 ("
                + QString::number(data.size()) + ").");

  foreach (QPointF point, data) {
    energies << point.x();
    whitebeam << point.y();
  }

  mu[Absorber::Empty] = QVector<double>(energies.size(), 0.0);

  loadMu(Absorber::Beryllium, mu[Absorber::Beryllium], energies);
  loadMu(Absorber::Graphite, mu[Absorber::Graphite], energies);
  loadMu(Absorber::Aluminium, mu[Absorber::Aluminium], energies);
  loadMu(Absorber::Copper, mu[Absorber::Copper], energies);
  loadMu(Absorber::Silver, mu[Absorber::Silver], energies);
  loadMu(Absorber::Gold, mu[Absorber::Gold], energies);
  loadMu(Absorber::Molybdenum, mu[Absorber::Molybdenum], energies);

  energies = energies * 0.001; // eV -> keV

  return true;

}




const bool FiltersGui::data_inited = init_data();

FiltersGui::FiltersGui(QWidget *parent) :
  ComponentGui(new Filters(parent), true, parent),
  ui(new Ui::Filters),
  motors(new QMotorStack(this))
{
  init();
}

FiltersGui::FiltersGui(Filters * flt, QWidget *parent) :
  ComponentGui(flt, false, parent),
  ui(new Ui::Filters),
  motors(new QMotorStack(this))
{
  init();
}


FiltersGui::~FiltersGui()
{
    delete ui;
}


void FiltersGui::init() {

  ui->setupUi(this);
  ui->advanced->hide();

  ui->SpectrumPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
  ui->SpectrumPlot->setAxisMaxMinor(QwtPlot::xBottom, 10);
  ui->SpectrumPlot->setAxisScale(QwtPlot::xBottom, energies.first(), energies.last());
  ui->SpectrumPlot->setAxisTitle(QwtPlot::xBottom, "Photon energy, keV");

  ui->SpectrumPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
  ui->SpectrumPlot->setAxisScale(QwtPlot::yLeft, spectrumMin, 1.0);

  QwtPlotGrid * SpectrumGrid = new QwtPlotGrid;
  SpectrumGrid->enableXMin(true);
  SpectrumGrid->enableYMin(true);
  SpectrumGrid->setMajPen(Qt::DashLine);
  SpectrumGrid->setMinPen(Qt::DotLine);
  SpectrumGrid->attach(ui->SpectrumPlot);

  QPen pen;

  wb_curve = new QwtPlotCurve("White Beam");
  wb_curve->setSamples(energies, whitebeam * ( 1/max(whitebeam) ));
  pen = QPen(QColor(0,0,0));
  pen.setWidth(8);
  wb_curve->setPen(pen);
  wb_curve->attach(ui->SpectrumPlot);

  current_curve = new QwtPlotCurve("Current Setup");
  pen = QPen(QColor(170,85,0));
  pen.setWidth(5);
  current_curve->setPen(pen);
  current_curve->attach(ui->SpectrumPlot);

  new_curve = new QwtPlotCurve("New Setup");
  pen = QPen(QColor(128,255,255));
  pen.setWidth(2);
  new_curve->setPen(pen);
  new_curve->attach(ui->SpectrumPlot);

  ui->SpectrumPlot->replot();

  ui->paddles_layout->setDirection(QBoxLayout::RightToLeft);
  QSvgWidget * arrow = new QSvgWidget(":/arrow.svg", this);
  arrow->setMinimumWidth(50);
  ui->paddles_layout->addWidget(arrow);

  foreach(Paddle* paddle, component()->paddles) {
    PaddleGui * paddleUI = new PaddleGui(paddle,this);
    motors->addMotor(paddle->motor(), true, true);
    ui->paddles_layout->addWidget(paddleUI);
    connect(paddleUI, SIGNAL(selectedChanged(int)), SLOT(updateSelection()));
    paddles << paddleUI;
  }

  motors->lock(true);
  ui->advancedLayout->addWidget(motors);

  connect(component(), SIGNAL(trainChanged(QList<Absorber::Foil>)), SLOT(updateTrain()));
  connect(component(), SIGNAL(motionStateChanged(bool)), SLOT(updateMotion(bool)));

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->gostop_pb, SIGNAL(clicked()), SLOT(onGoPressed()));
  connect(ui->reset_pb, SIGNAL(clicked()), SLOT(onResetPressed()));
  connect(ui->calibrate_pb, SIGNAL(clicked()), SLOT(onAutoCalibration()));

  updateTrain();
  updateSelection();

}







void FiltersGui::updateConnection(bool con) {

  ui->paddles_layout->setEnabled(con);
  ui->SpectrumPlot->setEnabled(con);
  ui->calibrate_pb->setEnabled(con);
  ui->reset_pb->setEnabled(con);
  ui->gostop_pb->setEnabled(con);

  ui->calibrate_pb->setText(con ? "Calibrate" : "Disconnected" );
  ui->reset_pb->setText(con ? "Reset" : "Disconnected");
  ui->gostop_pb->setText(con ? "Go" : "Disconnected");

  if (con) {
    updateMotion(component()->isMoving());
    updateTrain();
    QTimer::singleShot(0, this, SLOT(onResetPressed()));
  }

}

void FiltersGui::updateMotion(bool mov) {
  ui->gostop_pb->setText(mov ? "Stop" : "Go");
  ui->reset_pb->setEnabled(!mov);
  ui->calibrate_pb->setEnabled(!mov);
}

void FiltersGui::updateTrain() {
  QVector<double> cs;
  absorb(cs, component()->train());
  current_curve->setSamples(energies, cs);
  ui->SpectrumPlot->replot();
}

void FiltersGui::updateSelection() {
  QList<Absorber::Foil> new_train;
  foreach(PaddleGui* paddleUI, paddles)
    foreach( Absorber::Foil foil, paddleUI->selectedAbsorber().sandwich() ) {
      foil.second /= sin(paddleUI->component()->inclination());
      new_train << foil;
    }
  QVector<double> ns;
  absorb(ns, new_train);
  new_curve->setSamples(energies, ns);
  ui->SpectrumPlot->replot();
}


void FiltersGui::onGoPressed() {
  if (component()->isMoving())
    component()->stop(false);
  else if ( PsswDial::ask(this) )
    component()->setWindows(selectedWindows);
}

void FiltersGui::onResetPressed() {
  foreach(PaddleGui * paddleUI, paddles) {
    paddleUI->blockSignals(true);
    paddleUI->reset();
    paddleUI->blockSignals(false);
  }
  updateSelection();
}

void FiltersGui::onAdvancedControl() {
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

void FiltersGui::onAutoCalibration() {

  if ( ! PsswDial::ask(this) )
    return;

  ShutterFE::State inst(ShutterFE::stateS());
  if ( ! ShutterFE::setOpenedS(false,true) ) {
    warn("Can't close the FE shutter."
         " Calibration failed. Try to repeat or do it manually.", this);
    return;
  }

  QList<QCaMotor*> mlist;
  foreach(Paddle * paddle, component()->paddles)
    mlist << paddle->motor();

  QHash<QCaMotor*,double> positions;
  foreach(QCaMotor * mot, mlist) {
    positions[mot] = mot->get(); // get current positions
    mot->setUserLoLimit( -qAbs(mot->get()) - 500 ); // set software limits to something what is guaranteed to be beyond the real limit
    mot->goLimit(-1,false); // go to find the hardware limits
  }

  component()->wait_stop();

  bool allLimitsTriggered = true;
  foreach(QCaMotor * mot, mlist)
    allLimitsTriggered &= mot->getLoLimitStatus();
  if ( ! allLimitsTriggered ) {
    warn("Could not reach at least one of the motor limits."
         " Calibration failed. Try to repeat or do it manually.", this);
    return;
  }

  foreach(QCaMotor * mot, mlist) {
    mot->setUserPosition(0); // calibrate
    mot->setUserLoLimit(0); // restore software limits
    mot->goUserPosition(positions[mot], false); // return to the initial positions
  }

  ShutterFE::setOpenedS(inst == ShutterFE::OPENED);

}
