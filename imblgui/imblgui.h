#ifndef IMBLGUI_H
#define IMBLGUI_H

#include <QMainWindow>
#include "componentGui.h"


namespace Ui {
    class ImblGui;
}

class ImblGui : public QMainWindow {
  Q_OBJECT;

public:
  explicit ImblGui(QWidget *parent = 0);
  ~ImblGui();

private:
  Ui::ImblGui * ui;

private slots:
  void changeComponent(ComponentGui * component);
  void controlInWindow(ComponentGui * component);

};

#endif // IMBLGUI_H
