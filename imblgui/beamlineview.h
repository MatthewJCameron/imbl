#ifndef BEAMLINEVIEW_H
#define BEAMLINEVIEW_H

#include <QWidget>
#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QEvent>

#include "componentGui.h"


class ActiveArea : public QWidget {
  Q_OBJECT;

private:

  QImage imag;

  typedef enum {
    Normal,
    Active,
    Selected,
    Disabled,
    Alarm
  } Status;

  QHash<Status,QPixmap> pics;

  void prepare_pics();

  static const uint mintrans = 10, maxtrans = 110;
  bool attent, isSelected, isPassive;
  int trans;
  int timerID;

  uint newtrans();

signals:

  void pressed();
  void doubleclicked();

public:

  ActiveArea(const QString & _imag_name, bool _passive, QWidget *parent=0);
  ~ActiveArea();

  void selectme(bool sel) ;

  QSize sizeHint() const ;

  void Attention(bool _Att);
  void Attention();

public slots:

  void updateDescription(const QString & text);

private:

  void paintEvent(QPaintEvent *);
  void enterEvent ( QEvent * event);
  void leaveEvent ( QEvent * event);
  void timerEvent ( QTimerEvent * event);
  void mousePressEvent ( QMouseEvent * event) ;
  void mouseDoubleClickEvent ( QMouseEvent * event) ;


};





class BeamlineView : public QWidget {
    Q_OBJECT;

private:

  struct Element {
    ActiveArea * area;
    ComponentGui* component;
    float z; // z-position (from floor)
    float x; // x-position (from start of the drawing)
    float h; //height
    float w; //width
    Element(ActiveArea * _area, ComponentGui * _component,
            float _z, float _x, float _h, float _w=0);
  };

public:

  explicit BeamlineView(QWidget *parent = 0);
  ~BeamlineView();

  void show();

  ActiveArea * add(const QString & image_name, ComponentGui * controlW,
                   float _z, float _x, float _h, float _w=0);

  ActiveArea * add(const QString & image_name,
                   float _z, float _x, float _h, float _w=0);

  void select(const ActiveArea * selection);

  void paintEvent(QPaintEvent *);
  void changeEvent ( QEvent * event );
  bool eventFilter(QObject *target, QEvent *event);
  void resize (int h);

private:

  QList<Element> elements;
  float woh, maxX, maxY;
  QObject * oldparent;

private slots:

  void select();
  void settleInWindow();

signals:
  void changeSelection(ComponentGui * component);
  void wantLiveInWindow(ComponentGui * component);


};

#endif // BEAMLINEVIEW_H
