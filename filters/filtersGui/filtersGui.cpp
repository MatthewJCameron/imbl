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
  ui(new Ui::Paddle),
  buttonGroup(new QButtonGroup(this))
{

  ui->setupUi(this);

  connect(component()->motor(), SIGNAL(changedUserPosition(double)), SLOT(updateLabel()));
  connect(component(), SIGNAL(motionStateChanged(bool)), SLOT(updateMotionState(bool)));
  connect(component(), SIGNAL(windowChanged(int)), SLOT(updateWindow(int)));
  connect(component(), SIGNAL(limitStateChanged(bool)), SLOT(updateLabel()));

  const QList<Paddle::Window> & windows = component()->windows();
  for(int idx = 0 ; idx<windows.size() ; idx++ ) {
    QRadioButton * curbut = new QRadioButton(windows[idx].second.description(), this);
    curbut->setToolTip(QString::number(windows[idx].first));
    buttonGroup->addButton(curbut,idx);
    ui->verticalLayout->addWidget(curbut);
  }

  connect(buttonGroup, SIGNAL(buttonClicked(int)), SIGNAL(selectedChanged(int)));

  adjustSize();
  setMinimumSize(frameSize());
  updateConnection(component()->isConnected());

}

PaddleGui::~PaddleGui() {
  delete ui;
}

int PaddleGui::selectedWindow() const {
  return buttonGroup->checkedId();
}

void PaddleGui::selectWindow(int win) {
  if ( win >= 0 && win < buttonGroup->buttons().size() )
    buttonGroup->buttons()[win]->setChecked(true);
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
  foreach(QAbstractButton* but, buttonGroup->buttons() )
    but->setStyleSheet("");
  if ( win >= 0 )
    buttonGroup->button(win)->setStyleSheet("background-color: rgb(170, 85, 0);");
  updateLabel();
}

void PaddleGui::updateMotionState(bool mov) {
  foreach(QAbstractButton* but, buttonGroup->buttons() )
    but->setEnabled(!mov);
  updateLabel();
}


void PaddleGui::updateLabel() {
  QString text = component()->description() + "\n";
  if ( ! component()->isConnected() )
    text += "Disconnected";
  else if ( component()->isMoving() )
    text += QString::number(component()->motor()->getUserPosition());
  else if (component()->isOnLimit())
    text += "On limit";
  else if (component()->window() < 0)
    text += "Mispositioned!";
  else
    text += QString::number( component()->inclination() * 180 / M_PI ) + QString::fromUtf8("°");
  ui->label->setText(text);
}






static void loadData ( const QString & filename, QVector<double> & storage) {
  storage.clear();
  QFile funcf(filename);
  if ( ! funcf.open(QIODevice::ReadOnly) )
    throw_error("Could not open input file \"" + filename + "\".");
  QString rd;
  while ( ! funcf.atEnd()  &&   !(rd = funcf.readLine()).isEmpty() ) {
    bool ok;
    double rdnum = rd.toDouble(&ok);
    if ( ! ok )
      throw_error("Could not read float from \"" + rd + "\""
                  " in input file \"" + filename + "\".");
    storage << rdnum;
  }
  funcf.close();
}


const double spectrumMin=1.0e-11;
static QVector<double> energies;
static QVector<double> fields;
static QVector< QVector<double> > fluxdata;
static QHash< Absorber::Material, QVector<double> > mu;

void absorb(QVector<double> & inout, const QList<Absorber::Foil> & train) {
  const int size = energies.size();
  foreach(Absorber::Foil foil, train)
    if ( foil.first != Absorber::Empty && foil.second > 0.0 )
      for (int i=0 ; i<size ; i++)
        inout[i] *= exp( - mu[foil.first][i] * foil.second * 0.1);
  for (int i=0 ; i<size ; i++)
    if ( inout[i] < spectrumMin/10.0 )
      inout[i] = spectrumMin/10.0;
}


static bool init_data() {

  Q_INIT_RESOURCE(filters);

  loadData(":/SCMW.energy.dat", energies);
  loadData(":/SCMW.fields.dat", fields);

  const int ensize = energies.size();

  mu[Absorber::Empty] = QVector<double>(ensize, 0.0);
  foreach(Absorber::Material mat, Absorber::knownMaterials) {
    loadData(":/mu-" + materialName(mat) + ".dat", mu[mat]);
    if (mu[mat].size() != ensize)
      throw_error("File with the " + materialName(mat)
                  + " absorption coefficient has unexpected size ("
                  + QString::number(mu[mat].size()) + " != " + QString::number(ensize) + ").");
  }

  QFile funcf(":/SCMW.2Dflux.dat");
  if ( ! funcf.open(QIODevice::ReadOnly) )
    throw_error("Could not open input file \"" + funcf.fileName() + "\".");
  QString rd;

  while ( ! funcf.atEnd()  &&   !(rd = funcf.readLine()).isEmpty() ) {

    QStringList column = rd.split(" ");
    if (column.size() != ensize)
      throw_error("File with the flux data has unexpected size ("
                  + QString::number(column.size()) + " != " + QString::number(ensize) + ").");
    fluxdata << QVector<double>(ensize);

    bool ok;
    for (int enidx=0 ; enidx<ensize ; enidx++) {
      double flx = column[enidx].toDouble(&ok);
      if ( ! ok )
        throw_error("Could not read float from \"" + column[enidx] + "\""
                    " in input file \"" + funcf.fileName() + "\".");
      fluxdata.last()[enidx] = flx;
    }

  }

  funcf.close();

  if ( fluxdata.size() != fields.size() )
    throw_error("File with the flux data has an unexpected number of strings ("
                + QString::number(fluxdata.size()) + " != " + QString::number(fields.size()) + ").");

  return true;

}

const bool FiltersGui::data_inited = init_data();

FiltersGui::FiltersGui(QWidget *parent) :
  ComponentGui(new Filters(parent), true, parent),
  ui(new Ui::Filters),
  chooseMotors(new QWidget(this)),
  wigglerfield(new QEpicsPv("SR08ID01:GAP_MONITOR"))
{
  init();
}

FiltersGui::FiltersGui(Filters * flt, QWidget *parent) :
  ComponentGui(flt, false, parent),
  ui(new Ui::Filters),
  chooseMotors(new QWidget(this)),
  wigglerfield(new QEpicsPv("SR08ID01:GAP_MONITOR"))
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

  QVBoxLayout *calibrateLayout = new QVBoxLayout(chooseMotors);
  QLabel * label = new QLabel("Choose motors", chooseMotors);
  calibrateLayout->addWidget(label);
  foreach(Paddle* paddle, component()->paddles) {
    QCheckBox * chbk = new QCheckBox(chooseMotors);
    QCaMotor * mot = paddle->motor();
    chbk->setText(mot->getDescription());
    chooseMotorBoxes[mot] = chbk;
    calibrateLayout->addWidget(chbk);
  }

  ui->SpectrumPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
  ui->SpectrumPlot->setAxisMaxMinor(QwtPlot::xBottom, 10);
  ui->SpectrumPlot->setAxisScale(QwtPlot::xBottom, /*energies.first()*/ 10.0, energies.last());
  ui->SpectrumPlot->setAxisTitle(QwtPlot::xBottom, "Photon energy, keV");

  ui->SpectrumPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
  ui->SpectrumPlot->setAxisScale(QwtPlot::yLeft, 1.0e-9, 1.0);

  QwtPlotGrid * SpectrumGrid = new QwtPlotGrid;
  SpectrumGrid->enableXMin(true);
  SpectrumGrid->enableYMin(true);
  SpectrumGrid->setMajPen(Qt::DashLine);
  SpectrumGrid->setMinPen(Qt::DotLine);
  SpectrumGrid->attach(ui->SpectrumPlot);

  QPen pen;

  wb_curve = new QwtPlotCurve("White Beam");
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

  ui->motors->lock(true);
  foreach(Paddle* paddle, component()->paddles) {
    PaddleGui * paddleUI = new PaddleGui(paddle,this);
    ui->motors->addMotor(paddle->motor(), true, true);
    ui->paddles_layout->addWidget(paddleUI);
    connect(paddleUI, SIGNAL(selectedChanged(int)), SLOT(updatePlot()));
    paddles << paddleUI;
  }

  connect(component(), SIGNAL(trainChanged(QList<Absorber::Foil>)), SLOT(updatePlot()));
  connect(component(), SIGNAL(motionStateChanged(bool)), SLOT(updateMotion(bool)));

  connect(wigglerfield, SIGNAL(valueUpdated(QVariant)), SLOT(updateFields()));
  connect(wigglerfield, SIGNAL(connectionChanged(bool)), SLOT(updateFields()));

  connect(ui->advanced_pb, SIGNAL(clicked()), SLOT(onAdvancedControl()));
  connect(ui->gostop_pb, SIGNAL(clicked()), SLOT(onGoPressed()));
  connect(ui->reset_pb, SIGNAL(clicked()), SLOT(onResetPressed()));
  connect(ui->calibrate_pb, SIGNAL(clicked()), SLOT(onAutoCalibration()));
  connect(ui->fldPlot, SIGNAL(valueChanged(double)), SLOT(updatePlot()));
  connect(ui->fldLock, SIGNAL(toggled(bool)), SLOT(updateFields()));

  updatePlot();

  ui->additionalFilters->horizontalHeader()->setStretchLastSection(false);
  ui->additionalFilters->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
  ui->additionalFilters->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
  ui->additionalFilters->horizontalHeader()->setResizeMode(2,QHeaderView::Fixed);
  ui->additionalFilters->insertRow(0);
  QToolButton * addFilterBut = new QToolButton(ui->additionalFilters);
  addFilterBut->setText("add");
  connect(addFilterBut, SIGNAL(clicked()), SLOT(addFilter()));
  ui->additionalFilters->setCellWidget(0,0,addFilterBut);
  ui->additionalFilters->setSpan(0,0,1,3);

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
    updatePlot();
    QTimer::singleShot(0, this, SLOT(onResetPressed()));
  }

}

void FiltersGui::updateMotion(bool mov) {
  ui->gostop_pb->setText(mov ? "Stop" : "Go");
  ui->reset_pb->setEnabled(!mov);
  ui->calibrate_pb->setEnabled(!mov);
}


void FiltersGui::updatePlot() {

  const float field = ui->fldPlot->value();
  int fidx=0;
  while ( fidx < fields.size()  &&  fields[fidx] < field )
    fidx++;
  if (fidx >= fields.size())
    return;
  if ( fidx  &&  fields[fidx] - field > field - fields[fidx-1] )
    fidx--;
  QVector<double> spectrum = fluxdata[fidx];

  wb_curve->setSamples(energies, spectrum);

  QList<Absorber::Foil> add_train;
  for (int aidx=1 ; aidx < ui->additionalFilters->rowCount() ; aidx++) {
    QComboBox * mat = dynamic_cast<QComboBox*>(ui->additionalFilters->cellWidget(aidx, 0));
    Absorber::Material mt =
        mat ? Absorber::Material(mat->itemData(mat->currentIndex()).toInt()) : Absorber::Empty;
    QDoubleSpinBox * thick = dynamic_cast<QDoubleSpinBox*>(ui->additionalFilters->cellWidget(aidx, 1));
    add_train << Absorber::Foil( mt, thick ? thick->value() : 0.0 );
  }
  absorb(spectrum, add_train);

  QList<Absorber::Foil> new_train;
  selectedWindows.clear();
  foreach(PaddleGui* paddleUI, paddles) {
    selectedWindows << paddleUI->selectedWindow();
    foreach( Absorber::Foil foil, paddleUI->selectedAbsorber().sandwich() ) {
      foil.second /= sin(paddleUI->component()->inclination());
      new_train << foil;
    }
  }
  QVector<double> ns(spectrum);
  absorb(ns, new_train);
  new_curve->setSamples(energies, ns);

  QVector<double> cs(spectrum);
  absorb(cs, component()->train());
  current_curve->setSamples(energies, cs);

  ui->SpectrumPlot->replot();

}

void FiltersGui::updateFields() {
  if ( ! wigglerfield->isConnected() )
    return;
  const float field = wigglerfield->get().toDouble();
  ui->fldCur->setValue(field);
  if (ui->fldLock->isChecked())
    ui->fldPlot->setValue(field);
  ui->fldPlot->setEnabled(!ui->fldLock->isChecked());
}


void FiltersGui::addFilter(){

  ui->additionalFilters->insertRow(1);

  QComboBox * mat = new QComboBox(ui->additionalFilters);
  mat->setMaxCount(Absorber::knownMaterials.size());
  foreach (Absorber::Material mt, Absorber::knownMaterials)
    mat->addItem(materialName(mt), mt);
  connect(mat, SIGNAL(currentIndexChanged(int)), SLOT(updatePlot()));
  ui->additionalFilters->setCellWidget(1,0,mat);

  QDoubleSpinBox * thick = new QDoubleSpinBox(ui->additionalFilters);
  thick->setRange(0.0, 10000.0);
  thick->setValue(0.0);
  thick->setDecimals(1);
  thick->setSingleStep(0.1);
  thick->setSuffix("mm");
  connect(thick, SIGNAL(editingFinished()), SLOT(updatePlot()));
  ui->additionalFilters->setCellWidget(1,1,thick);

  QToolButton * rmbut = new QToolButton(ui->additionalFilters);
  rmbut->setText("-");
  connect(rmbut, SIGNAL(clicked()), SLOT(remFilter())); // remFilter is used only here
  ui->additionalFilters->setCellWidget(1,2,rmbut);

}


void FiltersGui::remFilter(){ // should be used only in one spot - search for it
  int idx=0;
  while ( idx < ui->additionalFilters->rowCount()  &&  sender() != ui->additionalFilters->cellWidget(idx, 2) )
    idx++;
  if(!idx)
    return;
  ui->additionalFilters->removeRow(idx);
}


void FiltersGui::onGoPressed() {

  if (component()->isMoving()) {
    component()->stop(false);
    return;
  }

  foreach(Paddle* paddle, component()->paddles)
    chooseMotorBoxes[paddle->motor()]->setText(paddle->motor()->getDescription());
  foreach(PaddleGui* paddleUI, paddles) {
    QCaMotor * mot = paddleUI->component()->motor();
    chooseMotorBoxes[mot]->setText(mot->getDescription());
    chooseMotorBoxes[mot]->setChecked
        ( paddleUI->selectedWindow() != paddleUI->component()->window() );
  }
  if ( ! PsswDial::askAddition(chooseMotors) )
    return;

  for (int pcur=0; pcur<paddles.size(); pcur++)
    if( ! chooseMotorBoxes[paddles[pcur]->component()->motor()]->isChecked()  )
      selectedWindows[pcur]=-1; // prevents deselected motors from travelling
  component()->setWindows(selectedWindows);
  updatePlot();

}


void FiltersGui::onResetPressed() {
  foreach(PaddleGui * paddleUI, paddles) {
    paddleUI->blockSignals(true);
    paddleUI->reset();
    paddleUI->blockSignals(false);
  }
  updatePlot();
}

void FiltersGui::onAdvancedControl() {
  if (ui->advanced->isVisible()) {
    ui->advanced->hide();
    ui->advanced_pb->setText("Show advanced control");
    ui->advanced_pb->setStyleSheet("");
  } else if ( PsswDial::ask(this) ) {
    ui->advanced->show();
    ui->advanced_pb->setText("CLICK here to hide advanced control");
    ui->advanced_pb->setStyleSheet("background-color: rgba(255, 0, 0,64);");
  }
}

void FiltersGui::onAutoCalibration() {

  foreach(Paddle* paddle, component()->paddles)
    chooseMotorBoxes[paddle->motor()]->setText(paddle->motor()->getDescription());

  foreach(QCheckBox *box, chooseMotorBoxes)
    box->setChecked(false);
  if (! PsswDial::askAddition(chooseMotors))
    return;

  QList<QCaMotor*> mlist;
  foreach(Paddle * paddle, component()->paddles)
    if (chooseMotorBoxes[paddle->motor()]->isChecked())
      mlist << paddle->motor();
  if (mlist.isEmpty())
    return;

  ShutterFE::State inst(ShutterFE::stateS());
  if ( ! ShutterFE::setOpenedS(false,true) ) {
    QMessageBox::warning(this, "Shutter error.",
                         "Can't close the FE shutter. Calibration failed. Try to repeat or do it manually.");
    return;
  }

  QHash<QCaMotor*,double> positions;
  foreach(QCaMotor * mot, mlist) {
    positions[mot] = mot->get(); // get current positions
    mot->setUserLoLimit( -qAbs(mot->get()) - 500 ); // set software limits to something what is guaranteed to be beyond the real limit
    mot->goLimit(-1,QCaMotor::STARTED); // go to find the hardware limits
  }

  //component()->wait_stop();

  bool allLimitsTriggered = true;
  foreach(QCaMotor * mot, mlist) {
    mot->wait_stop();
    allLimitsTriggered &= mot->getLoLimitStatus();
  }
  if ( ! allLimitsTriggered ) {
    QMessageBox::warning(this, "Calibration error.",
                         "Could not reach at least one of the motor limits."
                         " Calibration failed. Try to repeat or do it manually.");
    return;
  }

  foreach(QCaMotor * mot, mlist) {
    mot->setUserPosition(0); // calibrate
    mot->setUserLoLimit(0); // restore software limits
    mot->goUserPosition(positions[mot]); // return to the initial positions
  }

  ShutterFE::setOpenedS(inst == ShutterFE::OPENED);

}
