#include "qimbl.h"
#include "ui_qimbl.h"
#include "error.h"




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
static const QString red_style=
    "border-image: url(:/s_red.svg);"
    "color: rgb(0, 0, 0);";
static const QString green_style=
    "border-image: url(:/s_green.svg);"
    "color: rgb(0, 0, 0);";

static const QString shutter_open_string="Opened";
static const QString shutter_closed_string="Closed";



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
  shmrt(new MrtShutter(this)),
  slits(new HhlSlits(this)),
  filters(new Filters(this)),
  mono(new Mono(this))
{

  ui->setupUi(this);

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

  ui->rfcurrent->setSpecialValueText(nolink_string);
  connect(rfcurrent, SIGNAL(valueUpdated(QVariant)), SLOT(update_rfcurrent()));
  connect(rfcurrent, SIGNAL(connectionChanged(bool)), SLOT(update_rfcurrent()));

  ui->rfenergy->setSpecialValueText(nolink_string);
  connect(rfenergy, SIGNAL(valueUpdated(QVariant)), SLOT(update_rfenergy()));
  connect(rfenergy, SIGNAL(connectionChanged(bool)), SLOT(update_rfenergy()));

  ui->wigglergap->setSpecialValueText(nolink_string);
  connect(wigglergap, SIGNAL(valueUpdated(QVariant)), SLOT(update_wigglergap()));
  connect(wigglergap, SIGNAL(connectionChanged(bool)), SLOT(update_wigglergap()));

  hutches.insert( new Hutch(Hutch::H1A), qMakePair(ui->st1A, ui->label_1A) );
  hutches.insert( new Hutch(Hutch::H1B), qMakePair(ui->st1B, ui->label_1B) );
  hutches.insert( new Hutch(Hutch::H2A), qMakePair(ui->st2A, ui->label_2A) );
  hutches.insert( new Hutch(Hutch::H2B), qMakePair(ui->st2B, ui->label_2B) );
  hutches.insert( new Hutch(Hutch::TUN), qMakePair(ui->stT,  ui->label_T)  );
  hutches.insert( new Hutch(Hutch::H3A), qMakePair(ui->st3A, ui->label_3A) );
  hutches.insert( new Hutch(Hutch::H3B), qMakePair(ui->st3B, ui->label_3B) );
  foreach(Hutch * hut, hutches.keys()) {
    connect(hut, SIGNAL(enabledChanged(bool)), SLOT(update_hutches()));
    connect(hut, SIGNAL(stateChanged(Hutch::State)), SLOT(update_hutches()));
    connect(hut, SIGNAL(stackChanged(Hutch::StackColor)), SLOT(update_hutches()));
    connect(hut, SIGNAL(connectionChanged(bool)), SLOT(update_hutches()));
  }

  connect(shfe, SIGNAL(stateChanged(ShutterFE::State)), SLOT(update_shfe()));
  connect(shfe, SIGNAL(connectionChanged(bool)), SLOT(update_shfe()));

  connect(shmrt, SIGNAL(stateChanged(MrtShutter::State)), SLOT(update_shmrt()));
  connect(shmrt, SIGNAL(connectionChanged(bool)), SLOT(update_shmrt()));

  connect(slits, SIGNAL(geometryChanged(double,double,double,double)), SLOT(update_slits()));
  connect(slits, SIGNAL(limitStateChanged(HhlSlits::Limits)), SLOT(update_slits()));
  connect(slits, SIGNAL(motionStateChanged(bool)), SLOT(update_slits()));
  connect(slits, SIGNAL(connectionChanged(bool)), SLOT(update_slits()));

  if (filters->paddles.size() != 5) // shoud never happen
    throw_error("Unexpected number of filter foils ("
                + QString::number(filters->paddles.size())
                + " instead of 5).\n"
                "This should never happen, please report to the developer"   );
  connect(filters, SIGNAL(motionStateChanged(bool)), SLOT(update_filters()));
  connect(filters, SIGNAL(trainChanged(QList<Absorber::Foil>)), SLOT(update_filters()));
  connect(filters, SIGNAL(connectionChanged(bool)), SLOT(update_filters()));

  connect(mono, SIGNAL(motionChanged(bool)), SLOT(update_mono()));
  connect(mono, SIGNAL(energyChanged(double)), SLOT(update_mono()));
  connect(mono, SIGNAL(bend1Changed(double)), SLOT(update_mono()));
  connect(mono, SIGNAL(bend2Changed(double)), SLOT(update_mono()));
  connect(mono, SIGNAL(inBeamChanged(Mono::InOutPosition)), SLOT(update_mono()));
  connect(mono, SIGNAL(connectionChanged(bool)), SLOT(update_mono()));



  update_rfstat();
  update_rfcurrent();
  update_rfenergy();
  update_wigglergap();
  update_bl_status();
  update_eps_status();
  update_bl_mode();
  update_hutches();
  update_shfe();
  update_shmrt();
  update_slits();
  update_filters();
  update_mono();

}


Qimbl::~Qimbl() {
  delete ui;
}


static void updateOneHutch(Hutch * hut, QPair<QLabel*,QLabel*> & lab) {
  if ( ! hut->isConnected() ) {
    lab.first->setStyleSheet(nolink_style);
    lab.first->setText(nolink_string);
    lab.second->setEnabled(false);
  } else {
    lab.second->setEnabled(hut->isEnabled());
    switch (hut->stack()) {
    case Hutch::OFF : lab.first->setStyleSheet(gray_style); break;
    case Hutch::GREEN : lab.first->setStyleSheet(green_style); break;
    case Hutch::RED : lab.first->setStyleSheet(red_style); break;
    case Hutch::AMBER : lab.first->setStyleSheet(yellow_style); break;
    }
    switch (hut->state()) {
    case Hutch::OPEN : lab.first->setText("Open"); break;
    case Hutch::CLOSED : lab.first->setText("Closed"); break;
    case Hutch::LOCKED : lab.first->setText("Locked"); break;
    case Hutch::SEARCHED : lab.first->setText("Searched"); break;
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
  ui->st2A->setDisabled(false);
  ui->label_2A->setDisabled(false);
  ui->st2B->setDisabled(false);
  ui->label_2B->setDisabled(false);
  ui->stT->setDisabled(false);
  ui->label_T->setDisabled(false);
  ui->st3A->setDisabled(false);
  ui->label_3A->setDisabled(false);
  ui->st3B->setDisabled(false);
  ui->label_3B->setDisabled(false);
  if ( ! blmode1->isConnected() ||
       ! blmode2->isConnected() ||
       ! blmode3->isConnected() ) {
    ui->blMode->setStyleSheet(nolink_style);
    ui->blMode->setText(nolink_string);
  } else if ( 1 !=
              blmode1->get().toInt() +
              blmode2->get().toInt() +
              blmode3->get().toInt() ) {
    ui->blMode->setStyleSheet("color: rgb(255, 0, 0);");
    ui->blMode->setText("Inconsistent data");
  } else {
    ui->blMode->setStyleSheet("");
    if ( blmode1->get().toBool() ) {
      ui->blMode->setText("1");
      ui->st2A->setDisabled(true);
      ui->label_2A->setDisabled(true);
      ui->st2B->setDisabled(true);
      ui->label_2B->setDisabled(true);
      ui->stT->setDisabled(true);
      ui->label_T->setDisabled(true);
      ui->st3A->setDisabled(true);
      ui->label_3A->setDisabled(true);
      ui->st3B->setDisabled(true);
      ui->label_3B->setDisabled(true);
    } else if ( blmode2->get().toBool() ) {
      ui->blMode->setText("2");
      ui->stT->setDisabled(true);
      ui->label_T->setDisabled(true);
      ui->st3A->setDisabled(true);
      ui->label_3A->setDisabled(true);
      ui->st3B->setDisabled(true);
      ui->label_3B->setDisabled(true);
    } else
      ui->blMode->setText("3");
  }
}


void Qimbl::update_bl_status() {
  if ( ! bl_enabled->isConnected() ||
       ! bl_disabled->isConnected()) {
    ui->blSt->setStyleSheet(nolink_style);
    ui->blSt->setText(nolink_string);
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
    ui->epsSt->setStyleSheet(nolink_style);
    ui->epsSt->setText(nolink_string);
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
    ui->rfSt->setStyleSheet(nolink_style);
    ui->rfSt->setText(nolink_string);
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
    ui->rfcurrent->setStyleSheet(nolink_style);
    ui->rfcurrent->setValue(ui->rfcurrent->minimum());
  } else {
    ui->rfcurrent->setStyleSheet("");
    ui->rfcurrent->setValue(rfcurrent->get().toDouble());
  }
}

void Qimbl::update_rfenergy() {
  if ( ! rfenergy->isConnected() ) {
    ui->rfenergy->setStyleSheet(nolink_style);
    ui->rfenergy->setValue(ui->rfenergy->minimum());
  } else {
    ui->rfenergy->setStyleSheet("");
    ui->rfenergy->setValue(rfenergy->get().toDouble());
  }
}

void Qimbl::update_wigglergap() {
  if ( ! wigglergap->isConnected() ) {
    ui->wigglergap->setStyleSheet(nolink_style);
    ui->wigglergap->setValue(ui->wigglergap->minimum());
  } else {
    ui->wigglergap->setStyleSheet("");
    ui->wigglergap->setValue(wigglergap->get().toDouble());
  }
}


void Qimbl::update_shfe() {
  if ( ! shfe->isConnected() ) {
    ui->shfeSt->setStyleSheet(nolink_style);
    ui->shfeSt->setText(nolink_string);
  } else
    switch (shfe->state()) {
      case ShutterFE::CLOSED :
        ui->shfeSt->setStyleSheet(green_style);
        ui->shfeSt->setText(shutter_closed_string);
        break;
      case ShutterFE::OPENED :
        ui->shfeSt->setStyleSheet(red_style);
        ui->shfeSt->setText(shutter_open_string);
        break;
      case ShutterFE::BETWEEN :
        ui->shfeSt->setText(inprogress_string);
        break;
    }
}


void Qimbl::update_shmrt() {
  if ( ! shmrt->isConnected() ) {
    ui->shmrtSt->setStyleSheet(nolink_style);
    ui->shmrtSt->setText(nolink_string);
  } else if ( shmrt->progress() ) {
    ui->shmrtSt->setStyleSheet(red_style);
    ui->shmrtSt->setText("Running: " + QString::number(shmrt->progress()));
  } else
    switch (shmrt->state()) {
      case MrtShutter::CLOSED :
        ui->shmrtSt->setStyleSheet(green_style);
        ui->shmrtSt->setText(shutter_closed_string);
        break;
      case ShutterFE::OPENED :
        ui->shmrtSt->setStyleSheet(red_style);
        ui->shmrtSt->setText(shutter_open_string);
        break;
      case ShutterFE::BETWEEN :
        ui->shmrtSt->setText(inprogress_string);
        break;
    }
}


void Qimbl::update_slits() {
  if ( ! slits->isConnected() ) {
    ui->slitsStW->show();
    ui->slitsSt->setStyleSheet(nolink_style);
    ui->slitsSt->setText(nolink_string);
  } else if ( slits->isMoving() ) {
    ui->slitsStW->show();
    ui->slitsSt->setStyleSheet("");
    ui->slitsSt->setText(inprogress_string);
  } else if ( slits->limits() ) {
    ui->slitsStW->show();
    ui->slitsSt->setStyleSheet("");
    ui->slitsSt->setText("on limit(s)");
  } else {
    ui->slitsStW->hide();
    ui->slitsSt->setStyleSheet("");
    ui->slitsSt->setText("");
  }
  ui->slitsH->setValue(slits->height());
  ui->slitsW->setValue(slits->width());
  ui->slitsY->setValue(slits->hCenter());
  ui->slitsZ->setValue(slits->vCenter());
}


static void describePaddle(const Paddle * pad, QLabel * lab) {
  if ( pad->window() <0 ) { // misspositioned
    lab->setStyleSheet(red_style);
    lab->setText("Misspositioned!");
  } else if ( pad->isMoving() ) {
    lab->setStyleSheet("");
    lab->setText(QString::number(pad->motor()->getUserPosition()));
  } else {
    lab->setStyleSheet("");
    lab->setText(pad->absorber().description());
  }
}

void Qimbl::update_filters() {
  if ( ! filters->isConnected() ) {
    ui->filtersStW->show();
    ui->filtersSt->setStyleSheet(nolink_style);
    ui->filtersSt->setText(nolink_string);
  } else if ( filters->isMoving() ) {
    ui->filtersStW->show();
    ui->filtersSt->setStyleSheet("");
    ui->filtersSt->setText(inprogress_string);
  } else if ( filters->isMissPositioned() ) {
    ui->filtersStW->show();
    ui->filtersSt->setStyleSheet(red_style);
    ui->filtersSt->setText("Check it!");
  } else {
    ui->filtersStW->hide();
    ui->filtersSt->setStyleSheet("");
    ui->filtersSt->setText("");
  }
  describePaddle(filters->paddles[0], ui->filter1);
  describePaddle(filters->paddles[1], ui->filter2);
  describePaddle(filters->paddles[2], ui->filter3);
  describePaddle(filters->paddles[3], ui->filter4);
  describePaddle(filters->paddles[4], ui->filter5);
}


void Qimbl::update_mono() {
  if ( ! mono->isConnected() ) {
    ui->monoStW->show();
    ui->monoSt->setStyleSheet(nolink_style);
    ui->monoSt->setText(nolink_string);
  } else if ( mono->isMoving() ) {
    ui->monoStW->show();
    ui->monoSt->setStyleSheet("");
    ui->monoSt->setText(inprogress_string);
  } else if ( mono->inBeam() == Mono::BETWEEN ) {
    ui->monoStW->show();
    ui->monoSt->setStyleSheet(red_style);
    ui->monoSt->setText("Stopped in between");
  } else {
    ui->monoStW->hide();
    ui->monoSt->setStyleSheet("");
    ui->monoSt->setText("");
  }
  switch ( mono->inBeam() ) {
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
  ui->energy->setValue(mono->energy());
  switch (mono->diffraction()) {
    case Mono::Si111 :
      ui->hkl->setText("1,1,1");
      break;
    case Mono::Si311 :
      ui->hkl->setText("3,1,1");
      break;
  }
  ui->bend1->setValue(mono->bend1());
  ui->bend2->setValue(mono->bend2());
}









