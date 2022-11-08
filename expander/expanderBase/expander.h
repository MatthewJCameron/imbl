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
  static const QPair<double,double> energyRange;
  static const double theGradient;
  static const double theIntercept;

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
	//InOutPosition _inBeam;
	InOutPosition _tblInBeam;
	InOutPosition _expInBeam;
	static QHash<Motors,QCaMotor*> init_motors();
	double _dBragg; // delta of the gonio
	double _energy; // kev
	void initEnergy();
	bool _useDBragg;
public:
	explicit Expander(QObject *parent = 0);
	//inline InOutPosition inBeam() const {return _inBeam;}
	inline InOutPosition tblInBeam() const {return _tblInBeam;}
	inline InOutPosition expInBeam() const {return _expInBeam;}
	inline bool isMoving() const {return iAmMoving;}
	inline double dBragg() const {return _dBragg;}
	void wait_stop();
	inline double energy() const {return _energy; }


public slots:
	void setExpInBeam(bool val);
	void setTblInBeam(bool val);
	inline void expIn()  { setExpInBeam(true); }
	inline void expOut() { setExpInBeam(false); }
	void UpdateTblInOutStatus();
	void UpdateExpInOutStatus();
	inline void tblIn() { setTblInBeam(true); }
	inline void tblOut() { setTblInBeam(false); }
	void setEnergy(double enrg, bool keepDBragg=true);
  	void setDBragg(double val);
	void setUseDBragg(int val);
	void stop();

private slots:
	void updateMotion();
	void updateConnection();
	void updateDBragg();
	void updateEnergy();
	//void UpdateInOutStatus();

signals:
	//void inBeamChanged(Expander::InOutPosition);
	void tblInBeamChanged(Expander::InOutPosition);
	void expInBeamChanged(Expander::InOutPosition);
	void motionChanged(bool);
	void energyChanged(double);
  void dBraggChanged(double);
};

#endif //EXP_H
