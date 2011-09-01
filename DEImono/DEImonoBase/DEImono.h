#ifndef DEIMONO_H
#define DEIMONO_H

#include "component.h"
#include <qcamotor.h>

class DEImono : public Component {
  Q_OBJECT;

public:

  static QCaMotor * usMotor;
  static QCaMotor * dsMotor;
  static QCaMotor * zMotor;

  static const double totalCoeff;

private:

  static const double usArm;
  static const double usOffset;
  static const double dsArm;
  static const double dsOffset;

  static const double crystalLength;
  static const double crystalGap;

  bool iAmMoving;
  bool useDScrystal;
  double _energy;
  double _dsDisplacement; // murad
  double _zDisplacement; // mm

  static double posFromEnergy(double energy, const double & off, const double & arm, double displacement=0);

public:

  explicit DEImono(QObject *parent = 0);

  inline double energy() const { return _energy; }
  inline bool isMoving() const { return iAmMoving; }
  inline bool dsIsUsed() const { return useDScrystal; }
  inline double dsDisplacement() const { return _dsDisplacement;}
  inline double zDisplacement() const { return _zDisplacement; }

  static inline double usPosition(double energy){
    return posFromEnergy(energy, usOffset, usArm); }
  static inline double dsPosition(double energy, double displacement=0.0){
    return posFromEnergy(energy, dsOffset, displacement); }
  static double zPosition(double energy, double displacement = 0);
  static double energy(double usPos);

  void wait_stop();

public slots:

  void setEnergy(double val, bool keepDSdisplacement=true, bool keepZdisplacement=true);
  inline void setEnergy() { setEnergy(_energy, true, true); }
  void useDS(bool use);
  void setDSdisplacement(double val);
  void setZdisplacement(double val);

private slots:

  void updateMotion();
  void updateConnection();
  void updateUSposition();
  void updateDSdisplacement();
  void updateZdisplacement();

signals:

  void energyChanged(double);
  void motionChanged(bool);
  void stopped();
  void dsDisplacementChanged(double);
  void zDisplacementChanged(double);


};

#endif // DEIMONO_H
