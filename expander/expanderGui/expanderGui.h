#ifndef expanderGUI_H
#define expanderGUI_H

#include <QWidget>
#include <qmotorstack.h>
#include "componentGui.h"
#include "expander.h"

namespace Ui {
class ExpanderGui;
}

class ExpanderGui : public ComponentGui {
  Q_OBJECT;

private:

  Ui::ExpanderGui *ui;

  explicit ExpanderGui(Expander * expander, QWidget *parent = 0);
  
  void init();

public:

  explicit ExpanderGui(QWidget *parent = 0);
  ~ExpanderGui();

  inline ExpanderGui * duplicate(QWidget * parent=0) { return new ExpanderGui(component(), parent); }
  inline Expander * component() const {return static_cast<Expander*>( ComponentGui::component() );}

private slots:

  void onAdvancedControl();
  //void updateStatus();
  void updateConnection(bool con);
  void updateExpInOut(Expander::InOutPosition iopos);
  void updateTblInOut(Expander::InOutPosition iopos);

};




#endif // expanderGUI_H

