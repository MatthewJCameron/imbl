#ifndef EXP_H
#define EXP_H

#include "component.h"
#include <qcamotor.h>
#include <QHash>

class Expander : public Component {
	Q_OBJECT;

public:
	enum InOutPosition {INBEAM, OUTBEAM, BETWEEN, MOVING} ;
  static const QString pvBaseName;
  static const QString pvTableBaseName;

  enum Motors {
    tilt,
    slide,
    gonio,
    inOut,
    tbly,
    tblz
  };

  static const QHash<Motors,QCaMotor*> motors;

private:
	bool iAmMoving;
	InOutPosition _inBeam;
	static QHash<Motors,QCaMotor*> init_motors();

public:
	explicit Expander(QObject *parent = 0);
	inline InOutPosition inBeam() const {return _inBeam;}
	inline bool isMoving() const {return iAmMoving;}
	void wait_stop();
	void UpdateInOutStatus();

public slots:
	void setInBeam(bool val);
	inline void moveIn() { setInBeam(true); }
	inline void moveOut() { setInBeam(false); }
	void stop();

private slots:
	void updateMotion();
	void updateConnection();

signals:
	void inBeamChanged(Expander::InOutPosition);
	void motionChanged(bool);
};

#endif //EXP_H
