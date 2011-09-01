#ifndef HHLSLITS_H
#define HHLSLITS_H

#include "component.h"
#include <qcamotor.h>

#include <QSizeF>
#include <QPair>


class HhlSlits : public Component {
  Q_OBJECT;

public :

  static QCaMotor * vOpen;
  static QCaMotor * vPos;
  static QCaMotor * hLeft;
  static QCaMotor * hRight;

  static const double vOpenCalibre;
  static const double vPosCalibre;
  static const double hLeftCalibre;
  static const double hRightCalibre;

  static QPair<double, double>
  oc2pn(double openning, double center) {
    return qMakePair(openning/2+center, openning/2-center);
  }

  static QPair<double, double>
  pn2oc(double positive, double negative) {
    return qMakePair(positive+negative, (positive-negative)/2);
  }


public:
  enum Limit {
    ZPosHi   = 0x0200, ZPosLo   = 0x0100,
    HeightHi = 0x0040, HeightLo = 0x0020,
    RightHi  = 0x0010, RightLo  = 0x0004,
    LeftHi   = 0x0002, LeftLo   = 0x0001
  };
  Q_DECLARE_FLAGS(Limits, Limit);

private:

  static const QString pvBaseName;

  bool iAmMoving;
  Limits lms;
  double cl;
  double cr;
  double ch;
  double cz;

public:

  static const QSizeF fullbeam;

  explicit HhlSlits(QObject *parent = 0);

  inline bool isMoving() const {return iAmMoving;}

  inline Limits limits() const {return lms;}

  inline double top() const { return oc2pn(ch,cz).first;}
  inline double bottom() const {return oc2pn(ch,cz).second;}
  inline double height() const {return ch ;}
  inline double vCenter() const {return cz ;}
  inline double left()  const {return cl ;}
  inline double right()  const {return cr ;}
  inline double width()  const {return pn2oc(cl,cr).first ;}
  inline double hCenter()  const {return pn2oc(cl,cr).second;}

  void wait_stop();

signals:

  void motionStateChanged(bool isMov);
  void stopped();
  void limitStateChanged(HhlSlits::Limits lm);
  void geometryChanged(double _left, double _right, double _height, double _vcenter);

public slots:

  void stop(bool wait=false);

  void setGeometry(double _left, double _right, double _height, double _vcenter, bool wait=false);

  void setTop(double val, bool wait=false);
  void setBottom(double val, bool wait=false);
  void setHeight(double val, bool wait=false);
  void setVCenter(double val, bool wait=false);
  void setLeft(double val, bool wait=false);
  void setRight(double val, bool wait=false);
  void setWidth(double val, bool wait=false);
  void setHCenter(double val, bool wait=false);

protected slots:

  void updateConnection();
  void updateGeometry();
  void updateMotionState();
  void updateLimitState();

};

Q_DECLARE_OPERATORS_FOR_FLAGS(HhlSlits::Limits);

#endif // HHLSLITS_H
