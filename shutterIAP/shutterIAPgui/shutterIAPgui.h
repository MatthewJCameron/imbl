#ifndef ShutterIAPGUI_H
#define ShutterIAPGUI_H

#include <QTimer>
#include "componentGui.h"
#include "shutterIAP.h"

namespace Ui {
    class ShutterIAPgui;
}

class ShutterIAPgui : public ComponentGui {
  Q_OBJECT;

  explicit ShutterIAPgui(ShutterIAP * sht, QWidget *parent = 0);

public:

  explicit ShutterIAPgui(QWidget *parent = 0);
  ~ShutterIAPgui();

  inline ShutterIAPgui * duplicate(QWidget * parent=0) { return new ShutterIAPgui(component(), parent); }

  inline ShutterIAP * component() const {return static_cast<ShutterIAP*>( ComponentGui::component() );}

private:

  Ui::ShutterIAPgui *ui;

  const static QString styleTemplate;

protected slots:

  void updateState();
  void updateConnection(bool con);

};

#endif // ShutterIAPGUI_H
