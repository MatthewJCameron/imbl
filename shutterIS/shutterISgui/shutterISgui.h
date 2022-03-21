#ifndef SHUTTERISGUI_H
#define SHUTTERISGUI_H

#include <QTimer>
#include "componentGui.h"
#include "shutterIS.h"

namespace Ui {
    class ShutterISgui;
}

class ShutterISgui : public ComponentGui {
  Q_OBJECT;

  explicit ShutterISgui(ShutterIS * sht, QWidget *parent = 0);

public:

  explicit ShutterISgui(QWidget *parent = 0);
  ~ShutterISgui();

  inline ShutterISgui * duplicate(QWidget * parent=0) { return new ShutterISgui(component(), parent); }

  inline ShutterIS * component() const {return static_cast<ShutterIS*>( ComponentGui::component() );}

private:

  Ui::ShutterISgui *ui;

  const static QString styleTemplate;

protected slots:

  void updateStatuses();
  void updateConnection(bool con);

};

#endif // SHUTTERISGUI_H
