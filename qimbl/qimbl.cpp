#include "qimbl.h"
#include "ui_qimbl.h"
#include "error.h"
#include "columnresizer.h"
#include <stdio.h>



static const QString nolink_style=
    "border-image: url(:/s_black.svg);"
    "color: rgb(255, 128, 128);";
static const QString nolink_string="No link";
static const QString inprogress_string="Moving";
static const QString invalid_string="Invalid";
static const QString warning_string="WARNING";
static const QString alarm_string="ALARM";
static const QString ok_string="OK";



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

static const QString shutter_open_string="Open";
static const QString shutter_closed_string="Closed";
static const QString shInd_c_style =
    "background-color: rgb(255, 0, 0);"
    "color: rgb(0, 0, 0);";
static const QString shInd_o_style =
    "background-color: rgb(0, 255, 0);"
    "color: rgb(0, 0, 0);";


static void set_nolink_style(QLabel* lab) {
  lab->setStyleSheet(nolink_style);
  lab->setText(nolink_string);
}





Qimbl::Qimbl(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Qimbl),
  rfstat(new QEpicsPv("SR11BCM01:STORED_BEAM_STATUS")),
  rfcurrent(new QEpicsPv("SR11BCM01:CURRENT_MONITOR")),
  rfenergy(new QEpicsPv("SR00:BEAM_ENERGY_MONITOR")),
  wigglerfield(new QEpicsPv("SR08ID01:GAP_MONITOR")),
  bl_enabled(new QEpicsPv("SR08ID01PSS01:BL_ENABLE_STS")),
  bl_disabled(new QEpicsPv("SR08ID01PSS01:BL_DISABLE_STS")),
  eps_enabled(new QEpicsPv("SR08ID01PSS01:FES_EPS_ENABLE_STS")),
  eps_disabled(new QEpicsPv("SR08ID01PSS01:FES_EPS_DISABLE_STS")),
  shfe(new ShutterFE(this)),
  sh1A(new Shutter1A(this)),
  valve1(new Valve(1, this)),
  filters(new FiltersGui(this)),
  mono(new MonoGui(this)),
  shIS(new ShutterIS(this)),
  slidePos(new QEpicsPv("SR08ID01SST21:YTrans")),
  slidePosRBV(new QEpicsPv("SR08ID01SST21:YTrans.RBV")),
  expander(new ExpanderGui(this))
{

  //QEpicsPv::setDebugLevel(1);

  ui->setupUi(this);
  connect(ui->chooseComponent, SIGNAL(buttonClicked(QAbstractButton*)),SLOT(chooseComponent(QAbstractButton*)));

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

  connect(slidePos, SIGNAL(valueUpdated(QVariant)), SLOT(update_slidePos()));
  connect(slidePos, SIGNAL(connectionChanged(bool)), SLOT(update_slidePos()));

  connect(slidePosRBV, SIGNAL(valueUpdated(QVariant)), SLOT(update_slidePos()));
  connect(slidePosRBV, SIGNAL(connectionChanged(bool)), SLOT(update_slidePos()));


  connect(rfenergy, SIGNAL(valueUpdated(QVariant)), SLOT(update_rfenergy()));
  connect(rfenergy, SIGNAL(connectionChanged(bool)), SLOT(update_rfenergy()));

  connect(wigglerfield, SIGNAL(valueUpdated(QVariant)), SLOT(update_wigglerfield()));
  connect(wigglerfield, SIGNAL(connectionChanged(bool)), SLOT(update_wigglerfield()));

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
  connect(shfe, SIGNAL(enabledChanged(bool)), SLOT(update_shfe()));
  connect(ui->shfeControl, SIGNAL(clicked()), shfe, SLOT(toggle()));

  connect(sh1A, SIGNAL(ssStateChanged(Shutter1A::State)), SLOT(update_sh1A()));
  connect(sh1A, SIGNAL(psStateChanged(Shutter1A::State)), SLOT(update_sh1A()));
  connect(sh1A, SIGNAL(stateChanged(Shutter1A::State)), SLOT(update_sh1A()));
  connect(sh1A, SIGNAL(connectionChanged(bool)), SLOT(update_sh1A()));
  connect(sh1A, SIGNAL(enabledChanged(bool)), SLOT(update_sh1A()));
  connect(sh1A, SIGNAL(modeChanged(Shutter1A::Mode)), SLOT(update_bl_mode()));
  connect(sh1A, SIGNAL(connectionChanged(bool)), SLOT(update_bl_mode()));
  connect(ui->sh1AControl, SIGNAL(clicked()), sh1A, SLOT(toggle()));

  connect(ui->shmrt->component(), SIGNAL(stateChanged(MrtShutter::State)), SLOT(update_shmrt()));
  connect(ui->shmrt->component(), SIGNAL(connectionChanged(bool)), SLOT(update_shmrt()));
  connect(sh1A, SIGNAL(modeChanged(Shutter1A::Mode)), SLOT(update_shmrt()));

  connect(valve1, SIGNAL(stateChanged(Valve::State)), SLOT(update_valve_1()));
  connect(valve1, SIGNAL(connectionChanged(bool)), SLOT(update_valve_1()));

  connect(shIS, SIGNAL(stateChanged(ShutterIS::State)), SLOT(update_shIS()));
  connect(shIS, SIGNAL(connectionChanged(bool)), SLOT(update_shIS()));
  connect(shIS, SIGNAL(enabledChanged(bool)), SLOT(update_shIS()));
  connect(ui->shISControl, SIGNAL(clicked()), shIS, SLOT(toggle()));

  connect(ui->shISInOutButton, SIGNAL(clicked()), SLOT(MoveSlideToImagingShutter()));
  connect(ui->shMRTInOutButton, SIGNAL(clicked()), SLOT(MoveSlideToMRTShutter()));


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
  connect(mono->component(), SIGNAL(bend1obChanged(double)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(bend2obChanged(double)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(bend1ibChanged(double)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(bend2ibChanged(double)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(inBeamChanged(Mono::InOutPosition)), SLOT(update_mono()));
  connect(mono->component(), SIGNAL(connectionChanged(bool)), SLOT(update_mono()));

  ui->control->addWidget(expander);
  connect(expander->component(), SIGNAL(connectionChanged(bool)), SLOT(update_expander()));
  connect(expander->component(), SIGNAL(motionChanged(bool)), SLOT(update_expander()));
  connect(expander->component(), SIGNAL(tblInBeamChanged(Expander::InOutPosition)), SLOT(update_expander()));
  connect(expander->component(), SIGNAL(expInBeamChanged(Expander::InOutPosition)), SLOT(update_expander()));

  //Safety for FE
  connect(this,SIGNAL(_paddle4isNone(bool)),shfe,SLOT(paddle4isNone(bool)));
  connect(this,SIGNAL(_paddle5isNone(bool)),shfe,SLOT(paddle5isNone(bool)));
  connect(this,SIGNAL(_mono1isIn(bool)),    shfe,SLOT(mono1isIn(bool)));
  connect(this,SIGNAL(_expanderisIn(bool)), shfe,SLOT(expanderisIn(bool)));
  connect(this,SIGNAL(_bctTableisIn(bool)), shfe,SLOT(bctTableisIn(bool)));
  connect(this,SIGNAL(_shutterisMono(bool)), shfe,SLOT(shutterisMono(bool)));
  //safety for sh1A
  connect(this,SIGNAL(_expanderisIn(bool)), sh1A, SLOT(expanderisIn(bool)));
  connect(this,SIGNAL(_bctTableisIn(bool)), sh1A, SLOT(bctTableisIn(bool)));
  connect(this,SIGNAL(_expanderisIn(bool)), ui->shmrt->component(), SLOT(expanderisIn(bool)));
  connect(this,SIGNAL(_bctTableisIn(bool)), ui->shmrt->component(), SLOT(bctTableisIn(bool)));

  update_rfstat();
  update_rfcurrent();
  update_rfenergy();
  update_wigglerfield();
  update_bl_status();
  update_eps_status();
  update_bl_mode();
  update_hutches();
  update_shfe();
  update_sh1A();
  update_shmrt();
  update_filters();
  update_mono();
  update_valve_1();
  update_shIS();
  update_expander();

}


Qimbl::~Qimbl() {
  delete ui;
}


void Qimbl::chooseComponent(QAbstractButton* but) {
  if (but == ui->chooseFilters)
    ui->control->setCurrentWidget(filters);
  else if (but == ui->chooseMono)
    ui->control->setCurrentWidget(mono);
  else if (but == ui->chooseShutters)
    ui->control->setCurrentWidget(ui->shutters);
  else if (but == ui->chooseExpander)
    ui->control->setCurrentWidget(expander);
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

void Qimbl::update_valve_1() {
  if ( ! valve1->isConnected() ) {
    set_nolink_style(ui->shIndV1A_c);
    set_nolink_style(ui->shIndV1A_o);
    return;
  }
  switch (valve1->state()) {
    case Valve::OPENED :
      ui->shIndV1A_c->setStyleSheet("");
      ui->shIndV1A_c->setText("");
      ui->shIndV1A_o->setStyleSheet(shInd_o_style);
      ui->shIndV1A_o->setText(shutter_open_string);
      break;
    case Valve::CLOSED :
      ui->shIndV1A_c->setStyleSheet(shInd_c_style);
      ui->shIndV1A_c->setText(shutter_closed_string);
      ui->shIndV1A_o->setStyleSheet("");
      ui->shIndV1A_o->setText("");
      break;
    case Valve::MOVING :
      ui->shIndV1A_c->setStyleSheet(shInd_c_style);
      ui->shIndV1A_c->setText(inprogress_string);
      ui->shIndV1A_o->setStyleSheet(shInd_o_style);
      ui->shIndV1A_o->setText(inprogress_string);
      break;
    case Valve::INVALID :
      ui->shIndV1A_c->setStyleSheet(red_style);
      ui->shIndV1A_c->setText(invalid_string);
      ui->shIndV1A_o->setStyleSheet(red_style);
      ui->shIndV1A_o->setText(invalid_string);
      break;
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

  } else {

    switch (rfstat->get().toInt()) {
    case 0 : ui->rfSt->setStyleSheet(gray_style); break;
    case 1 : ui->rfSt->setStyleSheet(red_style); break;
    case 2 : ui->rfSt->setStyleSheet(green_style); break;
    default : ui->rfSt->setStyleSheet(""); break;
    }

    if (rfstat->getEnum().size() &&
             rfstat->get().toInt() < rfstat->getEnum().size()) // should always be true but is NOT
      ui->rfSt->setText( rfstat->getEnum()[rfstat->get().toInt()] );
    else {
      switch (rfstat->get().toInt()) {
      case 0 : ui->rfSt->setText("Unknown"); break;
      case 1 : ui->rfSt->setText("No Beam"); break;
      case 2 : ui->rfSt->setText("Beam available"); break;
      default : ui->rfSt->setText("Error"); break;
      }
      qDebug() << "BUG" << rfstat->getEnum() << rfstat->get().toInt();
    }

  }
}


void Qimbl::update_rfcurrent() {
  if ( ! rfcurrent->isConnected() ) {
    set_nolink_style(ui->rfcurrent);
  } else {
    ui->rfcurrent->setStyleSheet("");
    ui->rfcurrent->setText( QString::number(rfcurrent->get().toDouble(), 'f', 1) + "mA" );
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


void Qimbl::update_wigglerfield() {
  if ( ! wigglerfield->isConnected() ) {
    set_nolink_style(ui->wigglerfield);
  } else {
    ui->wigglerfield->setStyleSheet("");
    ui->wigglerfield->setText(QString::number(wigglerfield->get().toDouble(), 'f', 2) + "T");
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
        ui->shIndFE_o->setStyleSheet("");
        ui->shIndFE_o->setText("");
        ui->shfeControl->setText("Open");
        break;
      case ShutterFE::OPENED :
        ui->shfeSt->setStyleSheet(green_style);
        ui->shfeSt->setText(shutter_open_string);
        ui->shIndFE_c->setStyleSheet("");
        ui->shIndFE_c->setText("");
        ui->shIndFE_o->setStyleSheet(shInd_o_style);
        ui->shIndFE_o->setText(shutter_open_string);
        ui->shfeControl->setText("Close");
        break;
      case ShutterFE::BETWEEN :
        ui->shfeSt->setStyleSheet(red_style);
        ui->shfeSt->setText(inprogress_string);
        ui->shIndFE_c->setStyleSheet(shInd_c_style);
        ui->shIndFE_c->setText(inprogress_string);
        ui->shIndFE_o->setStyleSheet(shInd_c_style);
        ui->shIndFE_o->setText(inprogress_string);
        ui->shfeControl->setText("Close");
        break;
    }
    // ui->shfeControl->setEnabled(shfe->isEnabled());
    // if ( ! shfe->isEnabled() )
    //  ui->shfeControl->setText("Disabled");
  }
}


void Qimbl::update_bl_mode() {
  if ( ! sh1A->isConnected() )
    set_nolink_style(ui->blMode);
  else
    switch (sh1A->mode()) {
      case Shutter1A::INVALID :
        ui->blMode->setStyleSheet(red_style);
        ui->blMode->setText(invalid_string);
        ui->shmrt->setEnabled(false);
        ui->psShutterControlPlacer->addWidget(ui->sh1AControlWidget);
        ui->linked->setVisible(true);
        ui->linkedLeft->setVisible(true);
        ui->linkedRight->setVisible(true);
	emit _shutterisMono(false);
        break;
      case Shutter1A::MONO :
        ui->blMode->setStyleSheet("");
        ui->blMode->setText("Mono");
        ui->shmrt->setEnabled(false);
        ui->ssShutterControlPlacer->addWidget(ui->sh1AControlWidget);
        ui->linked->setVisible(false);
        ui->linkedLeft->setVisible(false);
        ui->linkedRight->setVisible(false);
	emit _shutterisMono(true);
        break;
      case Shutter1A::WHITE :
        ui->blMode->setStyleSheet("");
        ui->blMode->setText("White");
        ui->shmrt->setEnabled(false);
        ui->psShutterControlPlacer->addWidget(ui->sh1AControlWidget);
        ui->linked->setVisible(true);
        ui->linkedLeft->setVisible(true);
        ui->linkedRight->setVisible(true);
	emit _shutterisMono(false);
        break;
      case Shutter1A::MRT :
        ui->blMode->setStyleSheet("");
        ui->blMode->setText("MRT");
        ui->shmrt->setEnabled(true);
        ui->ssShutterControlPlacer->addWidget(ui->sh1AControlWidget);
        ui->linked->setVisible(false);
        ui->linkedLeft->setVisible(false);
        ui->linkedRight->setVisible(false);
	emit _shutterisMono(false);
        break;
    }
  update_sh1A();
}




void Qimbl::update_sh1A() {
  if ( ! sh1A->isConnected() ) {
    set_nolink_style(ui->shpsSt);
    set_nolink_style(ui->shssSt);
    set_nolink_style(ui->shIndPS_c);
    set_nolink_style(ui->shIndPS_o);
    set_nolink_style(ui->shIndSS_c);
    set_nolink_style(ui->shIndSS_o);
    return;
  }

  switch (sh1A->psState()) {
  case Shutter1A::CLOSED :
    ui->shpsSt->setStyleSheet( ( sh1A->mode() == Shutter1A::MONO ) ?
                               green_style : red_style);
    ui->shpsSt->setText(shutter_closed_string);
    ui->shIndPS_c->setStyleSheet(shInd_c_style);
    ui->shIndPS_c->setText(shutter_closed_string);
    ui->shIndPS_o->setStyleSheet("");
    ui->shIndPS_o->setText("");
    break;
  case Shutter1A::OPENED :
    ui->shpsSt->setStyleSheet( ( sh1A->mode() == Shutter1A::MONO ) ?
                               red_style : green_style );
    ui->shpsSt->setText(shutter_open_string);
    ui->shIndPS_c->setStyleSheet("");
    ui->shIndPS_c->setText("");
    ui->shIndPS_o->setStyleSheet(shInd_o_style);
    ui->shIndPS_o->setText(shutter_open_string);
    break;
  case Shutter1A::BETWEEN :
    ui->shpsSt->setStyleSheet(red_style);
    ui->shpsSt->setText(inprogress_string);
    ui->shIndPS_c->setStyleSheet(shInd_c_style);
    ui->shIndPS_c->setText(inprogress_string);
    ui->shIndPS_o->setStyleSheet(shInd_c_style);
    ui->shIndPS_o->setText(inprogress_string);
    break;
  }

  switch (sh1A->ssState()) {
  case Shutter1A::CLOSED :
    ui->shssSt->setStyleSheet(red_style);
    ui->shssSt->setText(shutter_closed_string);
    ui->shIndSS_c->setStyleSheet(shInd_c_style);
    ui->shIndSS_c->setText(shutter_closed_string);
    ui->shIndSS_o->setStyleSheet("");
    ui->shIndSS_o->setText("");
    break;
  case Shutter1A::OPENED :
    ui->shssSt->setStyleSheet(green_style);
    ui->shssSt->setText(shutter_open_string);
    ui->shIndSS_c->setStyleSheet("");
    ui->shIndSS_c->setText("");
    ui->shIndSS_o->setStyleSheet(shInd_o_style);
    ui->shIndSS_o->setText(shutter_open_string);
    break;
  case Shutter1A::BETWEEN :
    ui->shssSt->setStyleSheet(red_style);
    ui->shssSt->setText(inprogress_string);
    ui->shIndSS_c->setStyleSheet(shInd_c_style);
    ui->shIndSS_c->setText(inprogress_string);
    ui->shIndSS_o->setStyleSheet(shInd_c_style);
    ui->shIndSS_o->setText(inprogress_string);
    break;
  }

  switch (sh1A->state()) {
  case Shutter1A::CLOSED :
    ui->sh1AControl->setText("Open");
    break;
  case Shutter1A::OPENED :
  case Shutter1A::BETWEEN :
    ui->sh1AControl->setText("Close");
    break;
  }
  //ui->sh1AControl->setEnabled(sh1A->isEnabled());
  //if ( ! sh1A->isEnabled() )
  //  ui->sh1AControl->setText("Disabled");


}



void Qimbl::update_shmrt() {
  if ( ! ui->shmrt->component()->isConnected() ) {
    set_nolink_style(ui->shmrtSt);
    set_nolink_style(ui->shIndMRT_c);
    set_nolink_style(ui->shIndMRT_o);
  } else if ( ui->shmrt->component()->progress() ) {
    ui->shmrtSt->setStyleSheet(red_style);
    ui->shmrtSt->setText("Running: " + QString::number(ui->shmrt->component()->progress()));
  }

  if ( sh1A->mode() != Shutter1A::MRT ) {
    ui->shmrtSt->setStyleSheet(green_style);
    ui->shmrtSt->setText(shutter_open_string);
    ui->shIndMRT_c->setStyleSheet("");
    ui->shIndMRT_c->setText("");
    ui->shIndMRT_o->setStyleSheet(shInd_o_style);
    ui->shIndMRT_o->setText(shutter_open_string);
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
      ui->shIndMRT_c->setText("");
      ui->shIndMRT_o->setStyleSheet(shInd_o_style);
      ui->shIndMRT_o->setText(shutter_open_string);
      break;
    case MrtShutter::BETWEEN :
      ui->shmrtSt->setStyleSheet(red_style);
      ui->shmrtSt->setText(inprogress_string);
      ui->shIndMRT_c->setStyleSheet(shInd_c_style);
      ui->shIndMRT_c->setText(inprogress_string);
      ui->shIndMRT_o->setStyleSheet(shInd_c_style);
      ui->shIndMRT_o->setText(inprogress_string);
      break;
    case MrtShutter::EXPOSING :
      ui->shmrtSt->setStyleSheet(green_style);
      ui->shmrtSt->setText("Exposing");
      ui->shIndMRT_c->setStyleSheet(shInd_o_style);
      ui->shIndMRT_c->setText("Exposing");
      ui->shIndMRT_o->setStyleSheet(shInd_o_style);
      ui->shIndMRT_o->setText("Exposing");
      break;
    }
  }
}

void Qimbl::update_shIS() {
  if ( ! shIS->isConnected() ) {
    set_nolink_style(ui->shISSt);
    set_nolink_style(ui->shIndIS_c);
    set_nolink_style(ui->shIndIS_o);
  } else {
    switch (shIS->state()) {
      case ShutterIS::CLOSED :
        ui->shISSt->setStyleSheet(red_style);
        ui->shISSt->setText(shutter_closed_string);
        ui->shIndIS_c->setStyleSheet(shInd_c_style);
        ui->shIndIS_c->setText(shutter_closed_string);
        ui->shIndIS_o->setStyleSheet("");
        ui->shIndIS_o->setText("");
        ui->shISControl->setText("Open");
        break;
      case ShutterIS::OPENED :
        ui->shISSt->setStyleSheet(green_style);
        ui->shISSt->setText(shutter_open_string);
        ui->shIndIS_c->setStyleSheet("");
        ui->shIndIS_c->setText("");
        ui->shIndIS_o->setStyleSheet(shInd_o_style);
        ui->shIndIS_o->setText(shutter_open_string);
        ui->shISControl->setText("Close");
        break;
      case ShutterIS::BETWEEN :
        ui->shISSt->setStyleSheet(red_style);
        ui->shISSt->setText(inprogress_string);
        ui->shIndIS_c->setStyleSheet(shInd_c_style);
        ui->shIndIS_c->setText(inprogress_string);
        ui->shIndIS_o->setStyleSheet(shInd_c_style);
        ui->shIndIS_o->setText(inprogress_string);
        ui->shISControl->setText("Close");
        break;
    }
    // ui->shfeControl->setEnabled(shfe->isEnabled());
    // if ( ! shfe->isEnabled() )
    //  ui->shfeControl->setText("Disabled");
  }
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
  if (filters->component()->paddles[3]->absorber().description()=="None") emit _paddle4isNone(true);
  else emit _paddle4isNone(false); 
  if (filters->component()->paddles[4]->absorber().description()=="None") emit _paddle5isNone(true);
  else emit _paddle5isNone(false);
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
    ui->monoSt->setText("Between");
  } else {
    ui->monoStW->hide();
    ui->monoSt->setStyleSheet("");
    ui->monoSt->setText("");
  }
  switch ( mono->component()->inBeam() ) {
    case Mono::INBEAM :
      ui->monoPos->setStyleSheet("");
      ui->monoPos->setText("In beam");
      emit _mono1isIn(true);
      break;
    case Mono::OUTBEAM :
      ui->monoPos->setStyleSheet("");
      ui->monoPos->setText("Out of the beam");
      emit _mono1isIn(false);
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
  if (mono->component()->motors[Mono::Bragg2]->isMoving()) {
    ui->energy->setText("Moving");
    ui->hkl->setText("Moving");
  } else {
    ui->energy->setText(QString::number(mono->component()->energy(),'f',3));
    switch (mono->component()->diffraction()) {
      case Mono::Si111 :
        ui->hkl->setText("1,1,1");
        break;
      case Mono::Si311 :
        ui->hkl->setText("3,1,1");
        break;
    }
  }
  ui->bend1->setText(QString::number(mono->component()->bend1ob(),'f',2) + "/" +
                     QString::number(mono->component()->bend1ib(),'f',2));
  ui->bend2->setText(QString::number(mono->component()->bend2ob(),'f',2) + "/" +
                     QString::number(mono->component()->bend2ib(),'f',2));
}

void Qimbl::update_expander() {
  if ( ! expander->component()->isConnected() ) {
    set_nolink_style(ui->ExpInOut);
    set_nolink_style(ui->ExpSlide);
    set_nolink_style(ui->ExpTilt);
    set_nolink_style(ui->ExpGonio);
    set_nolink_style(ui->BCTZ);
    set_nolink_style(ui->BCTY);
    set_nolink_style(ui->TblInOrOut);
    set_nolink_style(ui->ExpInOrOut);
    return;
  }
  ui->ExpInOut->setStyleSheet("");
  ui->ExpSlide->setStyleSheet("");
  ui->ExpTilt->setStyleSheet("");
  ui->ExpGonio->setStyleSheet("");
  ui->BCTY->setStyleSheet("");
  ui->BCTZ->setStyleSheet("");
  ui->TblInOrOut->setStyleSheet("");
  ui->ExpInOrOut->setStyleSheet("");
  if (expander->component()->motors[Expander::inOut]->isMoving()) {
    ui->ExpInOut->setText("Moving");
    ui->ExpInOrOut->setText("Moving");
  } 
  else {
    ui->ExpInOut->setText(QString::number(expander->component()->motors[Expander::inOut]->getUserPosition(),'f',3) + " mm");
    if(expander->component()->expInBeam() == Expander::InOutPosition::INBEAM) {
      ui->ExpInOrOut->setText("In Beam");
      emit _expanderisIn(true);
      }
    else {
      ui->ExpInOrOut->setText("Out of Beam");
      emit _expanderisIn(false);
      }
  }
  if (expander->component()->motors[Expander::tilt]->isMoving()) {
    ui->ExpTilt->setText("Moving");
  } 
  else {
    ui->ExpTilt->setText(QString::number(expander->component()->motors[Expander::tilt]->getUserPosition(),'f',3) + " mm");
  }
  if (expander->component()->motors[Expander::slide]->isMoving()) {
    ui->ExpSlide->setText("Moving");
  } 
  else {
    ui->ExpSlide->setText(QString::number(expander->component()->motors[Expander::slide]->getUserPosition(),'f',3) + " mm");
  }
  if (expander->component()->motors[Expander::gonio]->isMoving()) {
    ui->ExpGonio->setText("Moving");
  } 
  else {
    ui->ExpGonio->setText(QString::number(expander->component()->motors[Expander::gonio]->getUserPosition(),'f',3) + " mm");
  }
  if (expander->component()->motors[Expander::tblz]->isMoving()) {
    ui->BCTZ->setText("Moving");
  } 
  else {
    ui->BCTZ->setText(QString::number(expander->component()->motors[Expander::tblz]->getUserPosition(),'f',3) + " mm");
  }
  if (expander->component()->motors[Expander::tbly]->isMoving()) {
    ui->BCTY->setText("Moving");
    ui->TblInOrOut->setText("Moving");
  } 
  else {
    ui->BCTY->setText(QString::number(expander->component()->motors[Expander::tbly]->getUserPosition(),'f',3) + " mm");
    if(expander->component()->tblInBeam() == Expander::InOutPosition::INBEAM) {
      ui->TblInOrOut->setText("In Beam");
      emit _bctTableisIn(true);
    }
    else {
      ui->TblInOrOut->setText("Out of Beam");
      emit _bctTableisIn(false);
    }
  }
}

void QLabelLine::paintEvent( QPaintEvent * event ) {
  QLabel::paintEvent(event);
  if ( ! text().isEmpty() )
    return;
  QPainter painter(this);
  QPen pen;
  pen.setWidth(3);
  //pen.setColor(Qt::yellow);
  painter.setPen(pen);
  painter.drawLine(0, height()/2, width(), height()/2);
}


void QLabelLineMono::paintEvent( QPaintEvent * event ) {
  QLabel::paintEvent(event);
  if ( ! text().isEmpty() )
    return;
  QPainter painter(this);
  QPen pen;
  pen.setWidth(3);
  //pen.setColor(Qt::yellow);
  painter.setPen(pen);
  painter.drawLine(0, height()*3/4, width(), height()*3/4);
  painter.drawLine(0, height()/4, width()/3, height()/4);
  painter.drawLine(width()/3, height()/4, width()*2/3, height()*3/4);
}



void QLabelLineTwo::paintEvent( QPaintEvent * event ) {
  QLabel::paintEvent(event);
  if ( ! text().isEmpty() )
    return;
  QPainter painter(this);
  QPen pen;
  pen.setWidth(3);
  //pen.setColor(Qt::yellow);
  painter.setPen(pen);
  painter.drawLine(0, rect().height()/4, width(), rect().height()/4);
  painter.drawLine(0, rect().height()*3/4, width(), rect().height()*3/4);

}


void LinkedLeft::paintEvent( QPaintEvent * event ) {
  QWidget::paintEvent(event);
  QPainter painter(this);
  QPen pen;
  pen.setWidth(1);
  //pen.setColor(Qt::yellow);
  painter.setPen(pen);
  painter.drawLine(width()/2, 0, width()/2, height()/2);
  painter.drawLine(width()/2, height()/2, width(), height()/2);
}

void LinkedRight::paintEvent( QPaintEvent * event ) {
  QWidget::paintEvent(event);
  QPainter painter(this);
  QPen pen;
  pen.setWidth(1);
  //pen.setColor(Qt::yellow);
  painter.setPen(pen);
  painter.drawLine(width()/2, 0, width()/2, height()/2);
  painter.drawLine(width()/2, height()/2, 0, height()/2);
}

void LinkedMiddle::paintEvent( QPaintEvent * event ) {
  QPainter painter(this);
  QPen pen;
  pen.setWidth(1);
  //pen.setColor(Qt::yellow);
  painter.setPen(pen);
  //painter.drawLine(0, height()/2, width(), height()/2);
  painter.drawLine(0, height()/2, width(), height()/2);
  QWidget::paintEvent(event);
}

void Qimbl::update_slidePos() {
  if ( ! slidePos->isConnected() ) {
    set_nolink_style(ui->ShutterSlidePos);
    ui->shMRTInOutButton->setDisabled(true);
    ui->shISInOutButton-> setDisabled(true);
    } 
  else {
    ui->ShutterSlidePos->setStyleSheet("");
    double currentPos = slidePosRBV->get().toDouble();
    ui->ShutterSlidePos->setText( QString::number(currentPos, 'f', 1) + " mm" );
    //std::cout << "currentPos is " << currentPos << std::endl;
    if ( 299.0 < currentPos ){
      ui->shMRTInOutButton->setText("Move IN");
      ui->shISInOutButton ->setText("Move OUT");
      ui->shMRTInOutButton->setEnabled(true);
      ui->shISInOutButton->setEnabled(true);
      }
    else if (currentPos < -299.0){
      ui->shMRTInOutButton->setText("Move OUT");
      ui->shISInOutButton ->setText("Move IN");
      ui->shMRTInOutButton->setEnabled(true);
      ui->shISInOutButton->setEnabled(true);
      }
    else if ( (-0.1 < currentPos+53.595) && (currentPos+53.595 < 0.1) ){
      ui->shMRTInOutButton->setText("Move IN");
      ui->shISInOutButton ->setText("Move IN");
      ui->shMRTInOutButton->setEnabled(true);
      ui->shISInOutButton->setEnabled(true);
      }
    else {
      ui->shMRTInOutButton->setText("Between");
      ui->shISInOutButton ->setText("Between");
      ui->shMRTInOutButton->setDisabled(true);
      ui->shISInOutButton-> setDisabled(true);
      }
    }
}

void Qimbl::MoveSlideToMRTShutter(){
  if ( ! slidePos->isConnected()){
    return;
    } 
  else {
    double currentPos = slidePosRBV->get().toDouble();
    if (currentPos > -299.0){
      slidePos->set(-300.0);
      ui->shMRTInOutButton->setText("Move OUT");
      }
    else {
      slidePos->set(-53.595);
      ui->shMRTInOutButton->setText("Move IN");
      }
    Qimbl::update_slidePos();
    }
}

void Qimbl::MoveSlideToImagingShutter(){
  if ( ! slidePos->isConnected()){
    return;
    } 
  else {
    double currentPos = slidePosRBV->get().toDouble();
    if (currentPos < 299.0){
      if ( sh1A->mode() != Shutter1A::MONO ) {
        QMessageBox::warning(0,"Cannot move ImagingShutter into position unless in mono", "To prevent you from moving imaging shutter into white or mrt beams, this function will not work unless shutter mode is mono. Change mode and repeat or move it manually at your peril. (OUT is 0.)");
        return;
        }
      else{
        slidePos->set(300.0);
        ui->shISInOutButton->setText("Move OUT");
        }
      }
    else {
      slidePos->set(-53.595);
      ui->shISInOutButton->setText("Move IN");
      }
    Qimbl::update_slidePos();
    }
}
