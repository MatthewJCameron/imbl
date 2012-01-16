#ifndef SHUTTER1AGUI_H
#define SHUTTER1AGUI_H

#include <QTimer>
#include "componentGui.h"
#include "shutter1A.h"

namespace Ui {
    class Shutter1Agui;
}

class Shutter1Agui : public ComponentGui {
  Q_OBJECT;

  explicit Shutter1Agui(Shutter1A * sht, QWidget *parent = 0);

public:

  explicit Shutter1Agui(QWidget *parent = 0);
  ~Shutter1Agui();

  inline Shutter1Agui * duplicate(QWidget * parent=0) { return new Shutter1Agui(component(), parent); }

  inline Shutter1A * component() const {return static_cast<Shutter1A*>( ComponentGui::component() );}

private:

  Ui::Shutter1Agui *ui;

  const static QString styleTemplate;

protected slots:

  void updateStatuses();
  void updateConnection(bool con);

};

#endif // SHUTTER1AGUI_H
