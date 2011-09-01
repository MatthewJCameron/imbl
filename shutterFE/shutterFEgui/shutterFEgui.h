#ifndef SHUTTERFEGUI_H
#define SHUTTERFEGUI_H

#include <QTimer>
#include "componentGui.h"
#include "shutterFE.h"

namespace Ui {
class ShutterFEgui;
}

class ShutterFEgui : public ComponentGui {
  Q_OBJECT;

  explicit ShutterFEgui(ShutterFE * sht, QWidget *parent = 0);

public:

  explicit ShutterFEgui(QWidget *parent = 0);
  ~ShutterFEgui();

  inline ShutterFEgui * duplicate(QWidget * parent=0) { return new ShutterFEgui(component(), parent); }

  inline ShutterFE * component() const {return static_cast<ShutterFE*>( ComponentGui::component() );}

private:

  Ui::ShutterFEgui *ui;

  const static QString styleTemplate;

protected slots:

  void updateState();
  void updateRelax();
  void updateConnection(bool con);

};

#endif // SHUTTERFEGUI_H
