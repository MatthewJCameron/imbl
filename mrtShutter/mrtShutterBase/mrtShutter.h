#ifndef MRTSHUTTER_H
#define MRTSHUTTER_H

#include "component.h"
#include <qtpv.h>
#include <QHash>
#include <QTimer>
#include <QTime>

class Shutter1A;

class MrtShutter : public Component {
  Q_OBJECT;

public:

  enum State {
    OPENED = 1,
    CLOSED = 0,
    BETWEEN = 2,
    EXPOSING = 3
  };

  enum ExposureMode {
    TIME=0,
    SOFT=1
//    HARD=2
  };



private:


  static const QString pvBaseName;
  static const QHash<QString,QEpicsPv*> pvs;
  static const QHash<QString,QEpicsPv*> init_static();
  static Shutter1A * shut1A; // needed to address the bug (see implimentation of start() member).

  /*
  QTimer dwellTimer;
  QTime startTime;
  */

  double _exposure;
  ExposureMode _expMode;
  double _cycle;
  int _repeats;
  int _progress; // 0 - not running, >0 - current repetiotion
  State _state;
  bool _canStart;
  bool _valuesOK;
  double _minRelax;

public:

  explicit MrtShutter(QObject * parent=0);

  inline double exposure() const {return _exposure;}
  inline ExposureMode exposureMode() const {return _expMode;}
  inline double cycle() const {return _cycle;}
  inline int repeats() const {return _repeats;}
  inline int progress() const {return _progress;}
  inline State state() const {return _state;}
  inline bool canStart() const {return _canStart;}
  inline bool valuesOK() const {return _valuesOK;}
  inline double minRelax() const {return _minRelax;}
  bool powerStatus();
  void tempFlags(bool * warn1=0, bool * warn2=0, bool * err1=0, bool * err2=0);
  void limitErrors(int * closeErr=0, int * openErr=0, int * restingErr=0);

public slots:

  void setOpened(bool opn);
  inline void open() {setOpened(true);}
  inline void close() {setOpened(false);}
  void start(bool beAwareOf1A=false);
  void trig(bool wait=false);
  void stop();
  void setExposure(double msec);
  void setExposureMode(ExposureMode val);
  void setCycle(double msec);
  void setRepeats(int val);
  void resetPower();
  void resetLimitErrors();
  void expanderisIn(bool newValue);
  void bctTableisIn(bool newValue);

private slots:
  void actual_start(); // needed to avoid a buggy situation: see more desc in the implementation of start().
  void acknowledgeSS(bool req);

protected slots:

  void updateConnection();
  void updateExposure();
  void updateExposureMode();
  void updateCycle();
  void updateRepeats();
  void updateProgress();
  void updateState();
  void updateCanStart();
  void updateValuesOK();
  void updateMinRelax();
  void updatePowerStatus();
  void updateLimitErrors();
  void updateTempFlags();


signals:

  void exposureChanged(double);
  void exposureModeChanged(MrtShutter::ExposureMode);
  void cycleChanged(double);
  void repeatsChanged(int);
  void minRelaxChanged(double);
  void progressChanged(int);
  void stateChanged(MrtShutter::State);
  void canStartChanged(bool);
  void valuesOKchanged(bool);
  void powerStatusChanged(bool pwr);
  void limitErrorsChanged(int closeErr, int openErr, int restingErr);
  void tempFlagsChanged(bool warn1, bool warn2, bool err1, bool err2);
  void _expanderisIn(bool avalue);
  void _bctTableisIn(bool avalue);

};

#endif // MRTSHUTTER_H

