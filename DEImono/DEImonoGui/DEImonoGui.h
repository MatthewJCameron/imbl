#ifndef DEIMONOGUI_H
#define DEIMONOGUI_H

#include <QtGui/QWidget>
#include <qmotorstack.h>
#include "componentGui.h"
#include "DEImono.h"

namespace Ui {
    class DEImonoGui;
}

class DEImonoGui : public ComponentGui {
  Q_OBJECT;

private:

    Ui::DEImonoGui *ui;
    QMotorStack * motors;

    static const double plankevXlighspeed;

    explicit DEImonoGui(DEImono * mono, QWidget *parent = 0);

    void init();

public:

    explicit DEImonoGui(QWidget *parent = 0);
    ~DEImonoGui();

    inline DEImonoGui * duplicate(QWidget * parent=0) { return new DEImonoGui(component(), parent); }
    inline DEImono * component() const {return static_cast<DEImono*>( ComponentGui::component() );}

private slots:

    void onAdvancedControl();
    void onShowCalibration();
    void onChangingMaterial();
    void onCalibrate();
    void onEnergyChange();
    void onEnergySet();
    void onTweakDS();
    void onTweakZ();

    void updateMotion(bool moving);
    void updateConnection(bool con);

};

#endif // DEIMONOGUI_H

