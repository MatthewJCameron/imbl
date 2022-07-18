#ifndef EXP_H
#define EXP_H

#include "component.h"
#include <qcamotor.h>
#include <QHash>

class Expander : public Component {
	Q_OBJECT;

public:
	enum InOutPosition {INBEAM, OUTBEAM, BETWEEN, MOVING} ;
	enum TblZPosition {INBEAM, OUTBEAM, BETWEEN, MOVING} ;
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
	TblZPosition _tblInBeam;
	static QHash<Motors,QCaMotor*> init_motors();

public:
	explicit Expander(QObject *parent = 0);
	inline InOutPosition inBeam() const {return _inBeam;}
	inline TblZPosition tblInBeam() const {return _tblInBeam;}
	inline bool isMoving() const {return iAmMoving;}
	void wait_stop();
	void UpdateInOutStatus();
	void UpdateTblInOutStatus();

public slots:
	void setInBeam(bool val);
	inline void moveIn() { setInBeam(true); }
	inline void moveOut() { setInBeam(false); }

	void setTblInBeam(bool val);
	inline void tblIn() { setTblInBeam(true); }
	inline void tblOut() { setTblInBeam(false); }

	void stop();

private slots:
	void updateMotion();
	void updateConnection();

signals:
	void inBeamChanged(Expander::InOutPosition);
	void tablInBeamChanged(Expander::TblZPosition);
	void motionChanged(bool);
};

#endif //EXP_H
