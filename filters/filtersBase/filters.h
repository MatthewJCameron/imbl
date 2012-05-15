#ifndef FILTERS_H
#define FILTERS_H

#include "component.h"
#include <qcamotor.h>


class Absorber {
public:
  enum Material {
    Empty,
    Beryllium,
    Graphite,
    Aluminium,
    Copper,
    Silver,
    Gold,
    Molybdenum,
    Carborundum
  };
  typedef QPair<Material, double> Foil;
private:
  QList<Foil> _sandwich;
public:

  inline Absorber() : _sandwich() {};
  Absorber(Material _material, double _thickness=0.0);
  Absorber(const QList<Foil> & sandw);
  inline const QList<Foil> & sandwich() const {return _sandwich;}
  inline bool isEmpty() const { return _sandwich.isEmpty(); }
  QString description() const;

};


QString materialName(Absorber::Material material);


class Paddle : public Component {
  Q_OBJECT;

public:

  typedef QPair<double,Absorber> Window;

private:

  static const double position_accuracy;

  QCaMotor * _motor;
  bool limitState;
  QList < Window > allWindows;
  int currentWindow;
  bool iAmMoving;
  double incl; // the angle between the beam and the paddle

public:

  Paddle(int order, const QString &  motorPV, const QList< Window > & _wins, double _incl, QObject * parent=0);

  inline QCaMotor * motor() const {return _motor;}
  inline bool isMoving() const {return iAmMoving;}
  inline bool isOnLimit() const {return limitState;}
  inline const QList<Window> & windows() const { return allWindows; }
  inline int window() const { return currentWindow; }
  inline const Absorber absorber(int win) const { return win < 0 || win >= allWindows.size() ?
          Absorber() : allWindows[win].second; }
  inline const Absorber absorber() const { return absorber(currentWindow); }
  inline double inclination() const {return incl;}

signals:

  void motionStateChanged(bool isMov);
  void limitStateChanged(bool lmStatus);
  void windowChanged(int win);
  void stopped();

public slots:

  inline void stop(bool wait=false) {
    if (!isConnected()) return;
    else _motor->stop(wait ? QCaMotor::STOPPED : QCaMotor::IMMIDIATELY);
  }

  void setWindow(int win);

private slots:

  void updateConnection();
  void updatePosiotion();
  void updateMotionState();
  void updateLimitState();

};




class Filters : public Component {
  Q_OBJECT;

public:
  static const QList<Paddle*> paddles;

private:

  static const QString pvBaseName;

  bool iAmMoving;
  QList<Absorber::Foil> _train;

public:

  Filters(QObject *parent = 0);

  inline const QList<Absorber::Foil> & train() const {return _train;}
  inline bool isMoving() const {return iAmMoving;}
  QList<int> windows() const;
  bool isMissPositioned() const;
  void wait_stop();

signals:
  void motionStateChanged(bool isMov);
  void stopped();
  void trainChanged(const QList<Absorber::Foil> & trn);

public slots:
  void stop(bool wait=false);
  void setWindows(const QList<int> & wins);

private slots:

  void updateConnection();
  void updateWindows();
  void updateMotionState();


};

#endif // FILTERS_H
