#ifndef MONO_H
#define MONO_H

#include "component.h"
#include <qcamotor.h>
#include <QHash>

class Mono : public Component {
  Q_OBJECT;

public:

  enum InOutPosition { INBEAM, OUTBEAM, BETWEEN, MOVING };

  static const double esinb = 1.9769289; // E * sin (ThetaB) = esinb;

private:

  static const double zDist = 2.0; // desired Z-separation of crystals;
  static const double zOut = 40.0; // Z1 when the mono is out of the beam;
  static const QPair<double,double> energyRange;

  enum Motors {
    Bragg1,
    Bragg2,
    Tilt1,
    Tilt2,
    Z1, // 0 when in beam
    Z2, // 0 when in beam
    Xdist,
    Bend1,
    Bend2
  };

  static const QHash<Motors,QCaMotor*> motors;
  static QHash<Motors,QCaMotor*> init_motors();

  bool iAmMoving;
  double _energy; // kev
  double _dBragg; // murad , displacement of the second crystal
  double _dX; // mm , X displacement of the stage
  double _dZ; // mm , Z displacement of the 2nd crystal
  double _tilt1; // mum
  double _tilt2; // mum
  double _bend1; // mum
  double _bend2; // mum
  InOutPosition _inBeam;

public:

  explicit Mono(QObject *parent = 0);

  inline double energy() const { return _energy; }
  inline double bragg() const { return motors[Bragg1]->getUserPosition() ; }
  inline double dBragg() const {return _dBragg;}
  inline double dX() const {return _dX;}
  inline double dZ() const {return _dZ;}
  inline double tilt1() {return _tilt1;}
  inline double tilt2() {return _tilt2;}
  inline double bend1() {return _bend1;}
  inline double bend2() {return _bend2;}
  inline InOutPosition inBeam() {return _inBeam;}

  inline bool isMoving() const { return iAmMoving; }
  inline QList<QCaMotor*> listMotors() const {return motors.values();}
  void wait_stop();

public slots:

  void setEnergy(double val, bool keepDBragg=true, bool keepDX=true);
  void setDBragg(double val);
  void setDX(double val);
  void setDZ(double val);
  void setTilt1(double val);
  void setTilt2(double val);
  void setBend1(double val);
  void setBend2(double val);
  void setInBeam(bool val);
  inline void moveIn() { setInBeam(true); }
  inline void moveOut() { setInBeam(false); }

  void stop();

private slots:

  void updateMotion();
  void updateConnection();

  void updateBragg1();
  void updateBragg2();
  void updateTilt1();
  void updateTilt2();
  void updateZ1();
  void updateZ2();
  void updateX();
  void updateBend1();
  void updateBend2();

signals:

  void motionChanged(bool);

  void energyChanged(double);
  void dBraggChanged(double);
  void dXChanged(double);
  void dZChanged(double);
  void tilt1Changed(double);
  void tilt2Changed(double);
  void bend1Changed(double);
  void bend2Changed(double);
  void inBeamChanged(Mono::InOutPosition);

};

#endif // MONO_H
