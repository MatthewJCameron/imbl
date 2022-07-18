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
	InOutPosition _tblInBeam;
	InOutPosition _expInBeam;
	static QHash<Motors,QCaMotor*> init_motors();

public:
	explicit Expander(QObject *parent = 0);
	inline InOutPosition inBeam() const {return _inBeam;}
	inline InOutPosition tblInBeam() const {return _tblInBeam;}
	inline InOutPosition expInBeam() const {return _expInBeam;}
	inline bool isMoving() const {return iAmMoving;}
	void wait_stop();


public slots:
	void setExpInBeam(bool val);
	inline void expIn()  { setExpInBeam(true); }
	inline void expOut() { setExpInBeam(false); }
	void UpdateTblInOutStatus();
	void UpdateExpInOutStatus();
	void setTblInBeam(bool val);
	inline void tblIn() { setTblInBeam(true); }
	inline void tblOut() { setTblInBeam(false); }

	void stop();

private slots:
	void updateMotion();
	void updateConnection();
	void UpdateInOutStatus();

signals:
	void inBeamChanged(Expander::InOutPosition);
	void tblInBeamChanged(Expander::InOutPosition);
	void expInBeamChanged(Expander::InOutPosition);
	void motionChanged(bool);
};

#endif //EXP_H
