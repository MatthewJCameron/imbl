#ifndef MRTSHUTTERGUI_H
#define MRTSHUTTERGUI_H

#include <QtGui/QWidget>
#include "mrtShutter.h"
#include "componentGui.h"

namespace Ui {
    class MrtShutterGui;
}

class MrtShutterGui : public ComponentGui {
  Q_OBJECT;

  explicit MrtShutterGui(MrtShutter * sht, QWidget *parent = 0);
  void init();

public:
  explicit MrtShutterGui(QWidget *parent = 0);
  ~MrtShutterGui();

  inline MrtShutterGui * duplicate(QWidget * parent=0) { return new MrtShutterGui(component(), parent); }

  inline MrtShutter * component() const {return static_cast<MrtShutter*>( ComponentGui::component() );}


private:

  Ui::MrtShutterGui *ui;
  bool wasProg;

protected slots:

  void updateConnection(bool con);
  void updateState(MrtShutter::State state);
  void updateValuesOK(bool ok);
  void updateProgress(int prog);
  void updateCanStart(bool can);
  void updateExposureMode(MrtShutter::ExposureMode mode);

  void onStartStop();
  void onOpenClose();
  void onExposureMode();
  void onTrig();



};

#endif // MRTSHUTTERGUI_H
