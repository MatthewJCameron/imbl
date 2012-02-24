#include "qimbl.h"
#include "ui_qimbl.h"
#include "error.h"
#include "columnresizer.h"




static const QString nolink_style=
    "border-image: url(:/s_black.svg);"
    "color: rgb(255, 128, 128);";
static const QString nolink_string="No link";
static const QString inprogress_string="Moving";


static const QString gray_style=
    "border-image: url(:/s_gray.svg);"
    "color: rgb(0, 0, 0);";
static const QString yellow_style=
    "border-image: url(:/s_yellow.svg);"
    "color: rgb(0, 0, 0);";
static const QString orange_style=
    "border-image: url(:/s_orange.svg);"
    "color: rgb(0, 0, 0);";
static const QString red_style=
    "border-image: url(:/s_red.svg);"
    "color: rgb(0, 0, 0);";
static const QString green_style=
    "border-image: url(:/s_green.svg);"
    "color: rgb(0, 0, 0);";

static const QString shutter_open_string="Opened";
static const QString shutter_closed_string="Closed";
static const QString shutter_through_string="<----------";
static const QString shInd_c_style="background-color: rgb(255, 0, 0);";
static const QString shInd_o_style="background-color: rgb(0, 255, 0);";


static void set_nolink_style(QLabel* lab) {
  lab->setStyleSheet(nolink_style);
  lab->setText(nolink_string);
}



const QStringList Qimbl::vacMonitors =
    ( QStringList()
      << "SR08ID01CCG01:PRESSURE_MONITOR"
      << "SR08ID01CCG02:PRESSURE_MONITOR"
      << "SR08FE01CCG01:PRESSURE_MONITOR"
      << "SR08FE01CCG02:PRESSURE_MONITOR" ) ;
const QStringList Qimbl::tempMonitors =
    ( QStringList()
      << "SR08FE01TES01:TEMPERATURE_MONITOR"
      << "SR08FE01TES02:TEMPERATURE_MONITOR") ;
const QStringList Qimbl::flowMonitors =
    ( QStringList()
      << "FLOW_MONITOR_1"
      << "FLOW_MONITOR_2") ;


Qimbl::Qimbl(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Qimbl),
  rfstat(new QEpicsPv("SR11BCM01:STORED_BEAM_STATUS")),
  rfcurrent(new QEpicsPv("SR11BCM01:CURRENT_MONITOR")),
  rfenergy(new QEpicsPv("SR00:BEAM_ENERGY_MONITOR")),
  wigglergap(new QEpicsPv("SR08ID01:GAP_MONITOR")),
  bl_enabled(new QEpicsPv("SR08ID01PSS01:BL_ENABLE_STS")),
  bl_disabled(new QEpicsPv("SR08ID01PSS01:BL_DISABLE_STS")),
  eps_enabled(new QEpicsPv("SR08ID01PSS01:FES_EPS_ENABLE_STS")),
  eps_disabled(new QEpicsPv("SR08ID01PSS01:FES_EPS_DISABLE_STS")),
  blmode1(new QEpicsPv("SR08ID01PSS01:BL_OPMODE1_STS")),
  blmode2(new QEpicsPv("SR08ID01PSS01:BL_OPMODE2_STS")),
  blmode3(new QEpicsPv("SR08ID01PSS01:BL_OPMODE3_STS")),
  shfe(new ShutterFE(this)),
  sh1A(new Shutter1A(this)),
  slits(new HhlSlitsGui(this)),
  filters(new FiltersGui(this)),
  mono(new MonoGui(this))
{

  ui->setupUi(this);
  connect(ui->chooseComponent, SIGNAL(buttonClicked(QAbstractButton*)),
          SLOT(chooseComponent(QAbstractButton*)));

  connect(bl_enabled, SIGNAL(valueUpdated(QVariant)), SLOT(update_bl_status()));
  connect(bl_enabled, SIGNAL(connectionChanged(bool)), SLOT(update_bl_status()));
  connect(bl_disabled, SIGNAL(valueUpdated(QVariant)), SLOT(update_bl_status()));
  connect(bl_disabled, SIGNAL(connectionChanged(bool)), SLOT(update_bl_status()));

  connect(eps_enabled, SIGNAL(valueUpdated(QVariant)), SLOT(update_eps_status()));
  connect(eps_enabled, SIGNAL(connectionChanged(bool)), SLOT(update_eps_status()));
  connect(eps_disabled, SIGNAL(valueUpdated(QVariant)), SLOT(update_eps_status()));
  connect(eps_disabled, SIGNAL(connectionChanged(bool)), SLOT(update_eps_status()));

  connect(rfstat, SIGNAL(valueUpdated(QVariant)), SLOT(update_rfstat()));
  connect(rfstat, SIGNAL(connectionChanged(bool)), SLOT(update_rfstat()));

  connect(rfcurrent, SIGNAL(valueUpdated(QVariant)), SLOT(update_rfcurrent()));
  connect(rfcurrent, SIGNAL(connectionChanged(bool)), SLOT(update_rfcurrent()));

  connect(rfenergy, SIGNAL(valueUpdated(QVariant)), SLOT(update_rfenergy()));
  connect(rfenergy, SIGNAL(connectionChanged(bool)), SLOT(update_rfenergy()));

  connect(wigglergap, SIGNAL(valueUpdated(QVariant)), SLOT(update_wigglergap()));
  connect(wigglergap, SIGNAL(connectionChanged(bool)), SLOT(update_wigglergap()));

  hutches.insert( new Hutch(Hutch::H1A), ui->st1A);
  hutches.insert( new Hutch(Hutch::H1B), ui->st1B);
  hutches.insert( new Hutch(Hutch::H2A), ui->st2A);
  hutches.insert( new Hutch(Hutch::H2B), ui->st2B);
  hutches.insert( new Hutch(Hutch::TUN), ui->stT );
  hutches.insert( new Hutch(Hutch::H3A), ui->st3A);
  hutches.insert( new Hutch(Hutch::H3B), ui->st3B);
  foreach(Hutch * hut, hutches.keys()) {
    connect(hut, SIGNAL(enabledChanged(bool)), SLOT(update_hutches()));
    connect(hut, SIGNAL(stateChanged(Hutch::State)), SLOT(update_hutches()));
    connect(hut, SIGNAL(stackChanged(Hutch::StackColor)), SLOT(update_hutches()));
    connect(hut, SIGNAL(connectionChanged(bool)), SLOT(update_hutches()));
  }

  connect(shfe, SIGNAL(stateChanged(ShutterFE::State)), SLOT(update_shfe()));
  connect(shfe, SIGNAL(connectionChanged(bool)), SLOT(update_shfe()));

  connect(sh1A, SIGNAL(stateChanged(Shutter1A::State)), SLOT(update_sh1A()));
  connect(sh1A, SIGNAL(connectionChanged(bool)), SLOT(update_sh1A()));

  connect(ui->shmrt->component(), SIGNAL(stateChanged(MrtShutter::State)), SLOT(update_shmrt()));
  connect(ui->shmrt->component(), SIGNAL(connectionChanged(bool)), SLOT(update_shmrt()));

  ui->control->addWidget(slits);
  connect(slits->component(), SIGNAL(geometryChanged(double,double,double,double)), SLOT(update_slits()));
  connect(slits->component(), SIGNAL(limitStateChanged(HhlSlits::Limits)), SLOT(update_slits()));
  connect(slits->component(), SIGNAL(motionStateChanged(bool)), SLOT(update_slits()));
  connect(slits->component(), SIGNAL(connectionChanged(bool)), SLOT(update_slits()));

  ui->control->addWidget(filters);
  if (filters->component()->paddles.size() != 5) // shoud never happen
    throw_error("Unexpected number of filter foils ("
                + QString::number(filters->component()->paddles.size())
                + " instead of 5).\n"
                "This should never happen, please report to the developer"   );
  connect(filters->component(), SIGNAL(motionStateChanged(bool)), SLOT(update_filters()));
  connect(filters->component(), SIGNAL(trainChanged(QList<Absorber::Foil>)), SLOT(update_filters()));
  connect(filters->component(), SIGNAL(connectionChanged(bool)), SLOT(update_filters()));

  ui->control->addWidget(mono);
  connect(mono->component(), SIGNAL(motionChanged(bool)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(energyChanged(double)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(bend1Changed(double)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(bend2Changed(double)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(inBeamChanged(Mono::InOutPosition)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(connectionChanged(bool)), SLOT(update_mono()));

  ColumnResizer* resizer;
  int row;

  row=1;
  resizer = new ColumnResizer(this);
  foreach(QString str, vacMonitors) {
    ValueBar * vb = new ValueBar(str);
    vb->setLogarithmic(true);
    vb->setMin(1.0e-09);
    ui->monitorsLayout->addWidget(vb, row++,0);
    vacBars << vb;
    connect(vb, SIGNAL(healthChenaged(ValueBar::Health)), SLOT(update_vacuum()));
    resizer->addWidgetsFromGridLayout(vb->internalLayout(),0);
  }
  row=1;
  foreach(QString str, tempMonitors) {
    ValueBar * vb = new ValueBar(str);
    ui->monitorsLayout->addWidget(vb, row++,1);
    tempBars << vb;
    connect(vb, SIGNAL(healthChenaged(ValueBar::Health)), SLOT(update_temperature()));
    resizer->addWidgetsFromGridLayout(vb->internalLayout(),0);
  }
  row=1;
  foreach(QString str, flowMonitors) {
    ValueBar * vb = new ValueBar(str);
    ui->monitorsLayout->addWidget(vb, row++,2);
    flowBars << vb;
    connect(vb, SIGNAL(healthChenaged(ValueBar::Health)), SLOT(update_flow()));
    resizer->addWidgetsFromGridLayout(vb->internalLayout(),0);
  }



  update_rfstat();
  update_rfcurrent();
  update_rfenergy();
  update_wigglergap();
  update_bl_status();
  update_eps_status();
  update_bl_mode();
  update_hutches();
  update_shfe();
  update_sh1A();
  update_shmrt();
  update_slits();
  update_filters();
  update_mono();
  update_vacuum();
  update_temperature();
  update_flow();


  ui->chooseMonitors->click();

}


Qimbl::~Qimbl() {
  delete ui;
}


void Qimbl::chooseComponent(QAbstractButton* but) {
  if (but == ui->chooseFilters)
    ui->control->setCurrentWidget(filters);
  else if (but == ui->chooseMono)
    ui->control->setCurrentWidget(mono);
  else if (but == ui->chooseSlits)
    ui->control->setCurrentWidget(slits);
  else if (but == ui->chooseMonitors)
    ui->control->setCurrentWidget(ui->monitors);
  else if (but == ui->chooseShutters)
    ui->control->setCurrentWidget(ui->shutters);
}


static void updateOneHutch(Hutch * hut, QLabel* lab) {
  if ( ! hut->isConnected() ) {
    set_nolink_style(lab);
  } else {
    switch (hut->stack()) {
    // Here inconsistence between the stack color
    // and *_style color is intentional
    case Hutch::OFF : lab->setStyleSheet(gray_style); break;
    case Hutch::GREEN : lab->setStyleSheet(red_style); break;
    case Hutch::RED : lab->setStyleSheet(green_style); break;
    case Hutch::AMBER : lab->setStyleSheet(orange_style); break;
    }
    switch (hut->state()) {
    case Hutch::OPEN : lab->setText("Open"); break;
    case Hutch::CLOSED : lab->setText("Closed"); break;
    case Hutch::LOCKED : lab->setText("Locked"); break;
    case Hutch::SEARCHED : lab->setText("Searched"); break;
    }
  }
}

void Qimbl::update_hutches() {
  Hutch * sndr = static_cast<Hutch*>(sender());
  if (sndr)
    updateOneHutch(sndr, hutches[sndr]);
  else
    foreach(Hutch * hut, hutches.keys())
      updateOneHutch(hut, hutches[hut]);
}


void Qimbl::update_bl_mode() {
  if ( ! blmode1->isConnected() ||
       ! blmode2->isConnected() ||
       ! blmode3->isConnected() ) {
    set_nolink_style(ui->blMode);
  } else if ( 1 !=
              blmode1->get().toInt() +
              blmode2->get().toInt() +
              blmode3->get().toInt() ) {
    ui->blMode->setStyleSheet("color: rgb(255, 0, 0);");
    ui->blMode->setText("Inconsistent data");
  } else {
    ui->blMode->setStyleSheet("");
    if ( blmode1->get().toBool() )
      ui->blMode->setText("1");
    else if ( blmode2->get().toBool() )
      ui->blMode->setText("2");
    else
      ui->blMode->setText("3");
  }
}


void Qimbl::update_bl_status() {
  if ( ! bl_enabled->isConnected() ||
       ! bl_disabled->isConnected()) {
    set_nolink_style(ui->blSt);
  } else if ( bl_enabled->get().toBool() &&
              ! bl_disabled->get().toBool() ) {
    ui->blSt->setStyleSheet(green_style);
    ui->blSt->setText("Enabled");
  } else if ( ! bl_enabled->get().toBool() &&
              bl_disabled->get().toBool() ) {
    ui->blSt->setStyleSheet(red_style);
    ui->blSt->setText("Disabled");
  } else {
    ui->blSt->setStyleSheet("color: rgb(255, 0, 0);");
    ui->blSt->setText("Inconsistent data");
  }
}


void Qimbl::update_eps_status() {
  if ( ! eps_enabled->isConnected() ||
       ! eps_disabled->isConnected()) {
    set_nolink_style(ui->epsSt);
  } else if ( eps_enabled->get().toBool() &&
              ! eps_disabled->get().toBool() ) {
    ui->epsSt->setStyleSheet(green_style);
    ui->epsSt->setText("Enabled");
  } else if ( ! eps_enabled->get().toBool() &&
              eps_disabled->get().toBool() ) {
    ui->epsSt->setStyleSheet(red_style);
    ui->epsSt->setText("Disabled");
  } else {
    ui->epsSt->setStyleSheet("color: rgb(255, 0, 0);");
    ui->epsSt->setText("Inconsistent data");
  }
}


void Qimbl::update_rfstat() {
  if ( ! rfstat->isConnected() ) {
    set_nolink_style(ui->rfSt);
  } else if ( ! rfstat->getEnum().size() ||
              rfstat->get().toInt() >= rfstat->getEnum().size() ) { // should never happen
    ui->rfSt->setStyleSheet("color: rgb(255, 0, 0);");
    ui->rfSt->setText("Error");
  } else {
    ui->rfSt->setText( rfstat->getEnum()[rfstat->get().toInt()] );
    switch (rfstat->get().toInt()) {
      case 0 : // unknown
        ui->rfSt->setStyleSheet(gray_style);
        break;
      case 1 : // no beam
        ui->rfSt->setStyleSheet(red_style);
        break;
      case 2 :
        ui->rfSt->setStyleSheet(green_style);
        break;
      default :
        ui->rfSt->setStyleSheet("");
        break;
    }
  }
}


void Qimbl::update_rfcurrent() {
  if ( ! rfcurrent->isConnected() ) {
    set_nolink_style(ui->rfcurrent);
  } else {
    ui->rfcurrent->setStyleSheet("");
    ui->rfcurrent->setText( QString::number(rfcurrent->get().toDouble(), 'f', 3) + "mA" );
  }
}


void Qimbl::update_rfenergy() {
  if ( ! rfenergy->isConnected() ) {
    set_nolink_style(ui->rfenergy);
  } else {
    ui->rfenergy->setStyleSheet("");
    ui->rfenergy->setText(QString::number(rfenergy->get().toDouble(), 'f', 3) + "GeV");
  }
}


void Qimbl::update_wigglergap() {
  if ( ! wigglergap->isConnected() ) {
    set_nolink_style(ui->wigglergap);
  } else {
    ui->wigglergap->setStyleSheet("");
    ui->wigglergap->setText(QString::number(wigglergap->get().toDouble(), 'f', 3) + "mm");
  }
}


void Qimbl::update_shfe() {
  if ( ! shfe->isConnected() ) {
    set_nolink_style(ui->shfeSt);
    set_nolink_style(ui->shIndFE_c);
    set_nolink_style(ui->shIndFE_o);
  } else {
    switch (shfe->state()) {
      case ShutterFE::CLOSED :
        ui->shfeSt->setStyleSheet(red_style);
        ui->shfeSt->setText(shutter_closed_string);
        ui->shIndFE_c->setStyleSheet(shInd_c_style);
        ui->shIndFE_c->setText(shutter_closed_string);
        ui->shIndFE_o->setStyleSheet(shInd_c_style);
        ui->shIndFE_o->setText("");
        ui->shfeControl->setText("Open");
        break;
      case ShutterFE::OPENED :
        ui->shfeSt->setStyleSheet(green_style);
        ui->shfeSt->setText(shutter_open_string);
        ui->shIndFE_c->setStyleSheet("");
        ui->shIndFE_c->setText(shutter_through_string);
        ui->shIndFE_o->setStyleSheet(shInd_o_style);
        ui->shIndFE_o->setText(shutter_open_string);
        ui->shfeControl->setText("Close");
        break;
      case ShutterFE::BETWEEN :
        ui->shfeSt->setText(inprogress_string);
        break;
    }
  }
}


void Qimbl::update_sh1A() {
  if ( ! sh1A->isConnected() ) {
    set_nolink_style(ui->shpsSt);
    set_nolink_style(ui->shssSt);
    set_nolink_style(ui->shIndPS_c);
    set_nolink_style(ui->shIndPS_o);
    set_nolink_style(ui->shIndSS_c);
    set_nolink_style(ui->shIndSS_o);
  } else {
    switch (sh1A->state()) {
      case ShutterFE::CLOSED :
        ui->shpsSt->setStyleSheet(red_style);
        ui->shpsSt->setText(shutter_closed_string);
        ui->shssSt->setStyleSheet(red_style);
        ui->shssSt->setText(shutter_closed_string);
        ui->shIndPS_c->setStyleSheet(shInd_c_style);
        ui->shIndPS_c->setText(shutter_closed_string);
        ui->shIndPS_o->setStyleSheet(shInd_c_style);
        ui->shIndPS_o->setText("");
        ui->shIndSS_c->setStyleSheet(shInd_c_style);
        ui->shIndSS_c->setText(shutter_closed_string);
        ui->shIndSS_o->setStyleSheet(shInd_c_style);
        ui->shIndSS_o->setText("");
        ui->sh1AControl->setText("Open");
        break;
      case ShutterFE::OPENED :
        ui->shpsSt->setStyleSheet(green_style);
        ui->shpsSt->setText(shutter_open_string);
        ui->shssSt->setStyleSheet(green_style);
        ui->shssSt->setText(shutter_open_string);
        ui->shIndPS_c->setStyleSheet("");
        ui->shIndPS_c->setText(shutter_through_string);
        ui->shIndPS_o->setStyleSheet(shInd_o_style);
        ui->shIndPS_o->setText(shutter_open_string);
        ui->shIndSS_c->setStyleSheet("");
        ui->shIndSS_c->setText(shutter_through_string);
        ui->shIndSS_o->setStyleSheet(shInd_o_style);
        ui->shIndSS_o->setText(shutter_open_string);
        ui->sh1AControl->setText("Close");
        break;
      case ShutterFE::BETWEEN :
        ui->shpsSt->setText(inprogress_string);
        ui->shssSt->setText(inprogress_string);
        break;
    }
  }
}



void Qimbl::update_shmrt() {
  if ( ! ui->shmrt->component()->isConnected() ) {
    set_nolink_style(ui->shmrtSt);
    set_nolink_style(ui->shIndMRT_c);
    set_nolink_style(ui->shIndMRT_o);
  } else if ( ui->shmrt->component()->progress() ) {
    ui->shmrtSt->setStyleSheet(red_style);
    ui->shmrtSt->setText("Running: " + QString::number(ui->shmrt->component()->progress()));
  } else {
    switch (ui->shmrt->component()->state()) {
      case MrtShutter::CLOSED :
        ui->shmrtSt->setStyleSheet(red_style);
        ui->shmrtSt->setText(shutter_closed_string);
        ui->shIndMRT_c->setStyleSheet(shInd_c_style);
        ui->shIndMRT_c->setText(shutter_closed_string);
        ui->shIndMRT_o->setStyleSheet(shInd_c_style);
        ui->shIndMRT_o->setText("");
        break;
      case MrtShutter::OPENED :
        ui->shmrtSt->setStyleSheet(green_style);
        ui->shmrtSt->setText(shutter_open_string);
        ui->shIndMRT_c->setStyleSheet("");
        ui->shIndMRT_c->setText(shutter_through_string);
        ui->shIndMRT_o->setStyleSheet(shInd_o_style);
        ui->shIndMRT_o->setText(shutter_open_string);
        break;
      case MrtShutter::BETWEEN :
        ui->shmrtSt->setText(inprogress_string);
        break;
    }
  }
}


void Qimbl::update_slits() {
  if ( ! slits->component()->isConnected() ) {
    ui->slitsStW->show();
    set_nolink_style(ui->slitsSt);
    set_nolink_style(ui->slitsH);
    set_nolink_style(ui->slitsW);
    set_nolink_style(ui->slitsY);
    set_nolink_style(ui->slitsZ);
    return;
  } else if ( slits->component()->isMoving() ) {
    ui->slitsStW->show();
    ui->slitsSt->setStyleSheet("");
    ui->slitsSt->setText(inprogress_string);
  } else if ( slits->component()->limits() ) {
    ui->slitsStW->show();
    ui->slitsSt->setStyleSheet("");
    ui->slitsSt->setText("on limit(s)");
  } else {
    ui->slitsStW->hide();
    ui->slitsSt->setStyleSheet("");
    ui->slitsSt->setText("");
  }
  ui->slitsH->setStyleSheet("");
  ui->slitsW->setStyleSheet("");
  ui->slitsY->setStyleSheet("");
  ui->slitsZ->setStyleSheet("");
  ui->slitsH->setText(QString::number(slits->component()->height(), 'f', 3) + "mm");
  ui->slitsW->setText(QString::number(slits->component()->width(), 'f', 3) + "mm");
  ui->slitsY->setText(QString::number(slits->component()->hCenter(), 'f', 3) + "mm");
  ui->slitsZ->setText(QString::number(slits->component()->vCenter(), 'f', 3) + "mm");
}


static void describePaddle(const Paddle * pad, QLabel * lab) {
  if ( ! pad->isConnected() ) {
    set_nolink_style(lab);
  } else if ( pad->window() <0 ) { // misspositioned
    lab->setStyleSheet(red_style);
    lab->setText("Misspositioned!");
  } else if ( pad->isMoving() ) {
    lab->setStyleSheet("");
    lab->setText(QString::number(pad->motor()->getUserPosition(), 'f', 3) + "mm");
  } else {
    lab->setStyleSheet("");
    lab->setText(pad->absorber().description());
  }
}

void Qimbl::update_filters() {
  if ( ! filters->component()->isConnected() ) {
    ui->filtersStW->show();
    set_nolink_style(ui->filtersSt);
  } else if ( filters->component()->isMoving() ) {
    ui->filtersStW->show();
    ui->filtersSt->setStyleSheet("");
    ui->filtersSt->setText(inprogress_string);
  } else if ( filters->component()->isMissPositioned() ) {
    ui->filtersStW->show();
    ui->filtersSt->setStyleSheet(red_style);
    ui->filtersSt->setText("Check it!");
  } else {
    ui->filtersStW->hide();
    ui->filtersSt->setStyleSheet("");
    ui->filtersSt->setText("");
  }
  describePaddle(filters->component()->paddles[0], ui->filter1);
  describePaddle(filters->component()->paddles[1], ui->filter2);
  describePaddle(filters->component()->paddles[2], ui->filter3);
  describePaddle(filters->component()->paddles[3], ui->filter4);
  describePaddle(filters->component()->paddles[4], ui->filter5);
}


void Qimbl::update_mono() {
  if ( ! mono->component()->isConnected() ) {
    ui->monoStW->show();
    set_nolink_style(ui->monoSt);
    set_nolink_style(ui->monoPos);
    set_nolink_style(ui->energy);
    set_nolink_style(ui->hkl);
    set_nolink_style(ui->bend1);
    set_nolink_style(ui->bend2);
    return;
  }
  ui->monoPos->setStyleSheet("");
  ui->energy->setStyleSheet("");
  ui->hkl->setStyleSheet("");
  ui->bend1->setStyleSheet("");
  ui->bend2->setStyleSheet("");
  if ( mono->component()->isMoving() ) {
    ui->monoStW->show();
    ui->monoSt->setStyleSheet("");
    ui->monoSt->setText(inprogress_string);
  } else if ( mono->component()->inBeam() == Mono::BETWEEN ) {
    ui->monoStW->show();
    ui->monoSt->setStyleSheet(red_style);
    ui->monoSt->setText("Stopped in between");
  } else {
    ui->monoStW->hide();
    ui->monoSt->setStyleSheet("");
    ui->monoSt->setText("");
  }
  switch ( mono->component()->inBeam() ) {
    case Mono::INBEAM :
      ui->monoPos->setStyleSheet("");
      ui->monoPos->setText("In beam");
      break;
    case Mono::OUTBEAM :
      ui->monoPos->setStyleSheet("");
      ui->monoPos->setText("Out of the beam");
      break;
    case Mono::BETWEEN :
      ui->monoPos->setStyleSheet(red_style);
      ui->monoPos->setText("Between");
      break;
    case Mono::MOVING :
      ui->monoPos->setStyleSheet("");
      ui->monoPos->setText("Moving");
      break;
  }
  ui->energy->setText(QString::number(mono->component()->energy()));
  switch (mono->component()->diffraction()) {
    case Mono::Si111 :
      ui->hkl->setText("1,1,1");
      break;
    case Mono::Si311 :
      ui->hkl->setText("3,1,1");
      break;
  }
  ui->bend1->setText(QString::number(mono->component()->bend1()));
  ui->bend2->setText(QString::number(mono->component()->bend2()));
}


void Qimbl::update_vacuum() {
  ValueBar::Health health = ValueBar::OK;
  foreach (ValueBar * vb, vacBars)
    if (vb->health() > health)
      health = vb->health();
  switch (health) {
    case ValueBar::OK :
      ui->vacSt->setStyleSheet(green_style);
      ui->vacSt->setText("All OK");
      break;
    case ValueBar::WARN :
      ui->vacSt->setStyleSheet(yellow_style);
      ui->vacSt->setText("Warn!");
      break;
    case ValueBar::ALARM :
      ui->vacSt->setStyleSheet(red_style);
      ui->vacSt->setText("Alarm!!!");
      break;
  }
}


void Qimbl::update_temperature() {
  ValueBar::Health health = ValueBar::OK;
  foreach (ValueBar * vb, tempBars)
    if (vb->health() > health)
      health = vb->health();
  switch (health) {
    case ValueBar::OK :
      ui->tempSt->setStyleSheet(green_style);
      ui->tempSt->setText("All OK");
      break;
    case ValueBar::WARN :
      ui->tempSt->setStyleSheet(yellow_style);
      ui->tempSt->setText("Warn!");
      break;
    case ValueBar::ALARM :
      ui->tempSt->setStyleSheet(red_style);
      ui->tempSt->setText("Alarm!!!");
      break;
  }
}


void Qimbl::update_flow() {
  ValueBar::Health health = ValueBar::OK;
  foreach (ValueBar * vb, flowBars)
    if (vb->health() > health)
      health = vb->health();
  switch (health) {
    case ValueBar::OK :
      ui->flowSt->setStyleSheet(green_style);
      ui->flowSt->setText("All OK");
      break;
    case ValueBar::WARN :
      ui->flowSt->setStyleSheet(yellow_style);
      ui->flowSt->setText("Warn!");
      break;
    case ValueBar::ALARM :
      ui->flowSt->setStyleSheet(red_style);
      ui->flowSt->setText("Alarm!!!");
      break;
  }
}






