#ifndef MONO_H
#define MONO_H

#include "component.h"
#include <qcamotor.h>
#include <QHash>




class Mono : public Component {
  Q_OBJECT;

public:

  enum InOutPosition { INBEAM, OUTBEAM, BETWEEN, MOVING };
  enum Diffraction { Si111, Si311 };

  static const QPair<double,double> energyRange;
  static const double maxEnergy111 = 100.0; // keV
  static const double minEnergy311 = 31.0; // keV

  enum Motors {
    Bragg1,
    Bragg2,
    Tilt1,
    Tilt2,
    Z1, // 0 when in beam
    Z2, // zDist when in beam
    Xdist,
    Bend1ob,
    Bend2ob,
    Bend1ib,
    Bend2ib
  };

  static const QHash<Motors,QCaMotor*> motors;

private:

  static const double standardZseparation = 25.0; // standard Z-separation of crystals;
  static const double zOut = -45.0; // Z1 when the mono is out of the beam;
  static const double alpha = 14.75; // asymmetry angle (deg) "+" for 111 "-" for 311

  static const double benderGap = 0.6;
  static const double bend1rt = 58.112;
  static const double bend2rt = 62.2;
  static const double bend1BraggCorrection = 0.00373;
  static const double bend2BraggCorrection = 0.00326;
  static const double bender1ibZero = 0.38;
  static const double bender1obZero = -0.30;
  static const double bender2ibZero = 0.0;
  static const double bender2obZero = 0.13;

  double bendR2X(double radius, Motors mot);
  double bendX2R(double pos, Motors mot);

  static QHash<Motors,QCaMotor*> init_motors();
  double motorAngle(double enrg, int crystal, Diffraction diff);

  bool iAmMoving;
  double _energy; // kev
  Diffraction _diff;
  double _dBragg; // displacement of the second crystal
  double _dX; // mm , X displacement of the stage
  double _zSeparation; // mm , Z nominal separation of the 2nd crystal
  double _dZ; // mm , The difference between Z nominal separation and actual Z.
  double b1ob;
  double b1ib;
  double b2ob;
  double b2ib;
  InOutPosition _inBeam;

public:

  explicit Mono(QObject *parent = 0);

  inline double energy() const {return _energy; }
  inline Diffraction diffraction() const {return _diff; }
  inline double dBragg() const {return _dBragg;}
  inline double dX() const {return _dX;}
  inline double zSeparation() const {return _zSeparation;}
  inline double zTweak() const {return _dZ;}
  inline double tilt1() const {return motors[Tilt1]->getUserPosition();}
  inline double tilt2() const {return motors[Tilt2]->getUserPosition();}
  inline double bend1ob() const {return b1ob;}
  inline double bend2ob() const {return b2ob;}
  inline double bend1ib() const {return b1ib;}
  inline double bend2ib() const {return b2ib;}
  inline InOutPosition inBeam() const {return _inBeam;}

  inline bool isMoving() const { return iAmMoving; }
  void wait_stop();

public slots:

  void setEnergy(double enrg, bool keepDBragg=true, bool keepDX=true);
  void setEnergy(double enrg, Mono::Diffraction diff, bool keepDBragg=true, bool keepDX=true);
  void setDBragg(double val);
  void setDX(double val);
  void setZseparation(double val, bool keepDZ);
  void setDZ(double val);
  void setTilt1(double val);
  void setTilt2(double val);
  void setBend1ob(double val);
  void setBend2ob(double val);
  void setBend1ib(double val);
  void setBend2ib(double val);
  void setInBeam(bool val);
  inline void makeConsistent() { setEnergy(energy(), diffraction()); }
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
  void updateBend1ob();
  void updateBend2ob();
  void updateBend1ib();
  void updateBend2ib();


signals:

  void motionChanged(bool);

  void energyChanged(double);
  void dBraggChanged(double);
  void dXChanged(double);
  void zSeparationChanged(double);
  void dZChanged(double);
  void tilt1Changed(double);
  void tilt2Changed(double);
  void bend1obChanged(double);
  void bend2obChanged(double);
  void bend1ibChanged(double);
  void bend2ibChanged(double);
  void inBeamChanged(Mono::InOutPosition);

};


double energy2bragg(double energy, Mono::Diffraction diff);


#endif // MONO_H
