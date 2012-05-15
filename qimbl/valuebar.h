#ifndef VALUEBAR_H
#define VALUEBAR_H

#include <QtGui/QWidget>
#include <QString>
#include <QHBoxLayout>
#include <qtpv.h>
#include <qwt_thermo.h>
#include <math.h>
#include <QApplication>
#include <QClipboard>

namespace Ui {
class ValueBar;
}



class ExtendedThermo : public QwtThermo {
  Q_OBJECT;
public :
  ExtendedThermo(QWidget * parent=0) :
    QwtThermo(parent),
    _lolo(NAN),
    _lo(NAN),
    _hi(NAN),
    _hihi(NAN)
  {}
  inline void setLL(double ll=NAN) {_lolo=ll; update();}
  inline void setL(double l=NAN) {_lo=l; update();}
  inline void setH(double h=NAN) {_hi=h; update();}
  inline void setHH(double hh=NAN) {_hihi=hh; update();}
  inline double lolo() {return _lolo;}
  inline double lo() {return _lo;}
  inline double hi() {return _hi;}
  inline double hihi() {return _hihi;}
protected:
  virtual void paintEvent( QPaintEvent * event );
private:
  double _lolo;
  double _lo;
  double _hi;
  double _hihi;
};



class ValueBar : public QWidget {
  Q_OBJECT;

public:

  enum Health {
    OK=0,
    WARN=1,
    ALARM=2
  };

  explicit ValueBar(const QString &_pv, QWidget *parent=0);
  ~ValueBar();

  inline Health health() const {return _health;}

  void setLogarithmic(bool lg);
  bool isLogarithmic();
  inline void setLoLo(double ll=NAN) {lolo=ll; updateParams();}
  inline void setLo(double l=NAN) {lo=l; updateParams();}
  inline void setHi(double h=NAN) {hi=h; updateParams();}
  inline void setHiHi(double hh=NAN) {hihi=hh; updateParams();}
  inline void setMin(double m=NAN) {min=m; updateParams();}
  inline void setMax(double m=NAN) {max=m; updateParams();}
  inline void setDescription(const QString & d=QString()) {desc=d; updateParams();};

  QGridLayout * internalLayout();

signals:

  void healthChenaged(ValueBar::Health hlth);

private slots:

  void updateValue();
  void updateParams();
  void updateConnection();
  void copyPV() {QApplication::clipboard()->setText(pv->pv());}

private:

  Ui::ValueBar *ui;
  QEpicsPv * pv;
  double lolo;
  double lo;
  double hi;
  double hihi;
  double min;
  double max;
  int prec;
  QString units;
  QString desc;
  Health _health;

  inline void setHealth(Health hlth) { emit healthChenaged(_health=hlth); }

};


#endif // VALUEBAR_H
