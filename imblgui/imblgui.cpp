#include "error.h"
#include "imblgui.h"
#include "ui_imblgui.h"

#include "beamlineview.h"
#include "shutterFEgui.h"
#include "hhlSlitsGui.h"
#include "filtersGui.h"
#include "shutter1Agui.h"
#include "mrtShutterGui.h"
#include "DEImonoGui.h"
#include <timescan.h>

#include <QDebug>

ImblGui::ImblGui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ImblGui)
{

    ui->setupUi(this);
    resize(800, 600);

    BeamlineView * blView = new BeamlineView(this);
    connect(blView, SIGNAL(changeSelection(ComponentGui *)),
            SLOT(changeComponent(ComponentGui *)));
    connect(blView, SIGNAL(wantLiveInWindow(ComponentGui *)),
            SLOT(controlInWindow(ComponentGui *)));

    ShutterFEgui * shFE = new ShutterFEgui(this);
    ui->stackedWidget->addWidget(shFE);
    blView->add(":/images/frontend.png", shFE, 1, 0, 1350);

    blView->add(":/images/fe-slit.png", 955, 327, 110);

    HhlSlitsGui * hhlS = new HhlSlitsGui(this);
    ui->stackedWidget->addWidget(hhlS);
    blView->add(":/images/slit.png", hhlS, 0, 388, 1564);

    QChartMX * vacMon = new QChartMX(this);
    vacMon->setInterval(0.5);
    vacMon->setPeriod(1800);
    vacMon->setContinious(true);
    vacMon->setAutoMin(false);
    vacMon->setMin(1.0e-09);
    vacMon->setAutoMax(false);
    vacMon->setMax(1.0e-05);
    vacMon->setLogarithmic(true);
    vacMon->setGridVisible(true);
    vacMon->setAutoName(false);
    vacMon->setSaveName("BL_VacMonitor_" +
                        QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
    vacMon->addSignal("SR08ID01CCG01:PRESSURE_MONITOR");
    vacMon->addSignal("SR08ID01CCG02:PRESSURE_MONITOR");
    vacMon->setControlCollapsed(true);
    vacMon->lock(true);
    vacMon->start();
    ComponentWidget * vacCmp = new ComponentWidget(vacMon,this);
    ui->stackedWidget->addWidget(vacCmp);

    blView->add(":/images/slit-IP.png", vacCmp, 276, 666, 362);

    blView->add(":/images/slit-filter.png", 955, 996, 110);

    FiltersGui * filterW = new FiltersGui(this);
    ui->stackedWidget->addWidget(filterW);
    blView->add(":/images/filter.png", filterW, 0, 1015, 2038);

    blView->add(":/images/filter-mono.png", 955, 1666, 110);

    blView->add(":/images/mono.png", 1, 1741, 1089);

    blView->add(":/images/mono-valve.png", 943, 3161, 449);

    blView->add(":/images/mono-valve-stop.png", 943, 3220, 146);

    Shutter1Agui * sh1a = new Shutter1Agui(this);
    ui->stackedWidget->addWidget(sh1a);
    blView->add(":/images/stop.png", sh1a, 1, 3266, 1114);

    blView->add(":/images/stop-mrt.png", 944, 3677, 146);

    blView->add(":/images/stop-mrt.png", 944, 3677, 146);

    MrtShutterGui * mrtW = new MrtShutterGui(this);
    ui->stackedWidget->addWidget(mrtW);
    blView->add(":/images/mrt.png", mrtW, 1, 3744, 1454);

    blView->add(":/images/mrt-IP.png", vacCmp, 391, 3815, 379);

    blView->add(":/images/mrt-ss.png", 944, 4032, 146);

    blView->add(":/images/ss.png", sh1a, 1, 4097, 1670);

    blView->add(":/images/ss-IP.png", vacCmp, 436, 4163, 364);

    blView->add(":/images/junction.png", 1, 4417, 1100);

    blView->add(":/images/1A1Bwall.png", 1, 4480, 2152);

    blView->add(":/images/rails1B.png", 0, 5040, 64);

    DEImonoGui * deiMono = new DEImonoGui(this);
    ui->stackedWidget->addWidget(deiMono);
    blView->add(":/images/DEImono.png", deiMono, 40, 6283, 1396);

    QMotorStack * mtstck;
    ComponentWidget * cmp;

    mtstck = new QMotorStack( QStringList()
                              << "SR08ID01:MTR12A"
                              << "SR08ID01:MTR12B"
                              << "SR08ID01:MTR12C"
                              << "SR08ID01:MTR12D"
                              << "SR08ID01:MTR12E"
                              << "SR08ID01:MTR12F"
                              << "SR08ID01:MTR12G"
                              << "SR08ID01:MTR12H"
                              << "SR08ID01:MTR11A"
                              << "SR08ID01:MTR11B"
                              << "SR08ID01:MTR11C"
                              << "SR08ID01:MTR11D"
                              << "SR08ID01:MTR11E"
                              << "SR08ID01:MTR11F"
                              << "SR08ID01:MTR11G"
                              << "SR08ID01:MTR11H");
    mtstck->lock(true);
    cmp = new ComponentWidget(mtstck,this);
    ui->stackedWidget->addWidget(cmp);
    blView->add(":/images/table1.png", cmp, 40, 5287, 510);

    mtstck = new QMotorStack( QStringList()
                              << "SR08ID01:MTR10A"
                              << "SR08ID01:MTR10B"
                              << "SR08ID01:MTR10C"
                              //<< "SR08ID01:MTR10D"
                              << "SR08ID01:MTR10E"
                              << "SR08ID01:MTR10F"
                              << "SR08ID01:MTR10G"
                              << "SR08ID01:MTR10H");
    mtstck->lock(true);
    cmp = new ComponentWidget(mtstck,this);
    ui->stackedWidget->addWidget(cmp);
    blView->add(":/images/table2.png", cmp, 40, 7727, 670);

    blView->add(":/images/ledstop1B.png", 3, 8603, 1221);

    ui->scroll_bl->setWidget(blView);

}

ImblGui::~ImblGui()
{
    delete ui;
}


void
ImblGui::changeComponent(ComponentGui * component) {
  if ( ! component || ui->stackedWidget->indexOf( component ) == -1 )
    warn("Zerro-widget or an widget not from the stack.");
  else
    ui->stackedWidget->setCurrentWidget(component);
}


void
ImblGui::controlInWindow(ComponentGui * component) {

  if ( ! component ) {
    warn("Zerro-widget.");
    return;
  }

  QDialog * cDial = new QDialog(this);
  QGridLayout * ll = new QGridLayout(cDial);
  ComponentGui * cc = component->duplicate( (QWidget*) cDial );
  ll->addWidget(cc,0,0,1,1);
  cDial->resize(cc->sizeHint());
  cDial->show();

  if ( dynamic_cast<ComponentWidget*>(component) ) {
    connect( dynamic_cast<ComponentWidget*>(component), SIGNAL(returned()),
             cDial, SLOT(close()) );
    connect( cDial, SIGNAL(finished(int)),
             dynamic_cast<ComponentWidget*>(component), SLOT(bringBackWidget()));
  }



}
