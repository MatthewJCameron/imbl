#include "error.h"
#include "beamlineview.h"

#include <QPainter>
#include <QBitmap>



ActiveArea::ActiveArea(const QString & _imag_name, bool _passive, QWidget *parent) :
  QWidget(parent),
  imag(_imag_name),
  attent(false),
  isSelected(false),
  isPassive(_passive),
  trans(mintrans),
  timerID(0)
{

  setMouseTracking(true);
  if ( ! isPassive )
    setCursor(Qt::PointingHandCursor);
  setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
  Attention(false);

  if (_imag_name.isEmpty() || imag.isNull() || imag.size() == QSize(0,0) )
    throw_error("No image file provided for the Active area or an empty image.", this);
  prepare_pics();
  update();

}


ActiveArea::~ActiveArea() {
  killTimer(timerID);
}


void
ActiveArea::prepare_pics() {

  if ( ! pics.isEmpty()
       && pics.contains(Normal)
       && pics[Normal].size() == size() )
    return; // everything's ready

  QImage szimag = imag.scaled
      (size(), Qt::IgnoreAspectRatio ,Qt::SmoothTransformation);
  QImage tmpimag;
  QPixmap tmpix;
  QPainter painter;

  tmpix = QPixmap::fromImage(szimag);
  setMask(tmpix.mask());

  pics[Normal] = tmpix;

  tmpix = QPixmap::fromImage(szimag);
  painter.begin(&tmpix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(rect(),QColor(255, 255, 255, 50));
  painter.end();
  pics[Active] = tmpix;

  // gray out
  tmpimag = szimag;
  for (int x = 0 ; x < tmpimag.width() ; ++x )
    for (int y = 0 ; y < tmpimag.height() ; ++y ) {
      int val = qGray(tmpimag.pixel(x,y));
      tmpimag.setPixel(x,y,qRgb(val, val, val));
    }
  tmpix = QPixmap::fromImage(tmpimag);
  pics[Disabled] = tmpix;

  tmpix = QPixmap::fromImage(szimag);
  painter.begin(&tmpix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(rect(),QColor(255, 215, 0, 100));
  painter.end();
  pics[Selected] = tmpix;


  tmpix = QPixmap::fromImage(szimag);
  painter.begin(&tmpix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(rect(),QColor(255, 0, 0, 50));
  painter.end();
  pics[Alarm] = tmpix;

}

uint
ActiveArea::newtrans() {
  if ( abs(trans) <= (int) mintrans ) trans = mintrans + 1;
  else if ( abs(trans) >= (int) maxtrans ) trans = - maxtrans + 1;
  else trans++;
  return abs(trans);
}


void
ActiveArea::selectme(bool sel) {
  if (! isPassive)
    isSelected = sel;
  update();
}


QSize
ActiveArea::sizeHint() const {
  return imag.isNull()  ?  QSize()  :  imag.size();
}

void
ActiveArea::Attention(bool _att){
  if ( isPassive || attent == _att )
    return;
  attent = _att;
  if (attent) {
    timerID = startTimer(10);
  } else {
    killTimer(timerID);
    timerID=0;
  }
  update();
}

void
ActiveArea::Attention(){
  Attention (!attent);
}

void ActiveArea::updateDescription(const QString &text) {
  QString desc;
  if (sender())
    desc = sender()->objectName() + ":\n\n";
  setToolTip(desc + text);
}


void
ActiveArea::paintEvent(QPaintEvent *) {
  prepare_pics();
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
  if ( isPassive || ! isEnabled() )
    painter.drawPixmap(rect(),pics[Disabled]);
  else if ( attent ) {
    QPixmap tmpix=pics[Alarm];
    QPainter pixpaint;
    pixpaint.begin(&tmpix);
    pixpaint.setRenderHint(QPainter::Antialiasing, true);
    pixpaint.fillRect(rect(),QColor(255, 0, 0, newtrans()));
    pixpaint.end();
    painter.drawPixmap(rect(),tmpix);
  } else if ( isSelected )
    painter.drawPixmap(rect(),pics[Selected]);
  else if ( underMouse() )
    painter.drawPixmap(rect(),pics[Active]);
  else
    painter.drawPixmap(rect(),pics[Normal]);
}


void
ActiveArea::enterEvent ( QEvent * event){
  if (isPassive)
    return;
  QWidget::enterEvent(event);
  update();
}


void
ActiveArea::leaveEvent ( QEvent * event){
  if (isPassive)
    return;
  QWidget::leaveEvent(event);
  QWidget::releaseMouse();
  update();
}

void
ActiveArea::timerEvent ( QTimerEvent * event){
  QWidget::timerEvent(event);
  update();
}

void
ActiveArea::mousePressEvent ( QMouseEvent * event) {
  if (isPassive)
    return;
  QWidget::mousePressEvent(event);
  emit pressed();
}

void
ActiveArea::mouseDoubleClickEvent ( QMouseEvent * event) {
  if (isPassive)
    return;
  QWidget::mouseDoubleClickEvent(event);
  emit doubleclicked();
}







BeamlineView::Element::Element (ActiveArea * _area, ComponentGui * _component,
                                float _z, float _x, float _h, float _w) :
  area (_area),
  component(_component),
  z(_z),
  x(_x),
  h(_h),
  w(_w)
{

  if(!area)
    throw_error("Zero active area of the BeamlineView element.");

  if ( h<0 || w<0 || x<0 || z<0 )
    throw_error("Active area of the BeamlineView element"
                " has at least one negative coordinate.", area);
  if ( w == 0 )
    w = h * area->sizeHint().width() / area->sizeHint().height();
  if ( w==0 || h==0 )
    throw_error("Active area of the BeamlineView element has at least one zero-size.",
                area);

  if (component)
    connect(component->component(), SIGNAL(descriptionChanged(QString)),
            area, SLOT(updateDescription(QString)));

}


BeamlineView::BeamlineView(QWidget *parent) :
  QWidget(parent),
  woh(0),
  maxX(0),
  maxY(0)
{
  setVisible(false);
}

BeamlineView::~BeamlineView() {
  foreach(Element element, elements)
    delete element.area;
}

void
BeamlineView::select() {
  select( (const ActiveArea *) sender() );
}

void
BeamlineView::select(const ActiveArea * selection) {
  foreach(Element element, elements)
    if ( ! element.component ) {
      // Do nothing
    } else if (element.area != selection)
      element.area->selectme(false);
    else {
      element.area->selectme(true);
      emit changeSelection(element.component);
    }
}


void
BeamlineView::settleInWindow() {
  foreach(Element element, elements)
    if (element.area == sender() && element.component)
      emit wantLiveInWindow(element.component);
}


void
BeamlineView::show(){
  if (elements.empty())
    throw_error("No components in the BeamlineView.");
  else
    setVisible(true);
}


ActiveArea *
BeamlineView::add(const QString & image_name, ComponentGui * component,
                  float _z, float _x, float _h, float _w) {

  ActiveArea * area = new ActiveArea(image_name, false, this);
  elements  << Element(area, component, _z, _x, _h, _w);

  connect( area, SIGNAL(pressed()), SLOT(select()) );
  connect( area, SIGNAL(doubleclicked()), SLOT(settleInWindow()) );

  foreach(Element element, elements) {
    float cX = element.w + element.x;
    if (maxX < cX) maxX = cX;
    float cY = element.h + element.z;
    if (maxY < cY) maxY = cY;
  }
  woh = maxX/maxY;

  select(area);
  return area;

}


ActiveArea *
BeamlineView::add(const QString & image_name,
                  float _z, float _x, float _h, float _w) {

  ActiveArea * area = new ActiveArea(image_name, true, this);
  elements << Element(area, 0, _z, _x, _h, _w);
  area->setEnabled(false);

  foreach(Element element, elements) {
    float cX = element.w + element.x;
    if (maxX < cX) maxX = cX;
    float cY = element.h + element.z;
    if (maxY < cY) maxY = cY;
  }
  woh = maxX/maxY;
  return area;

}


void
BeamlineView::paintEvent(QPaintEvent *) {
  foreach(Element element, elements) {
    int ch = height(), cw = width();
    element.area->setGeometry
        ( cw - ch * (element.x + element.w) / maxY,
          ch - ch * (element.z + element.h) / maxY,
          element.w * ch / maxY,
          element.h *ch / maxY );
  }
}


void
BeamlineView::changeEvent ( QEvent * event ) {
  if (event->type() == QEvent::ParentChange) {
    removeEventFilter(oldparent);
    oldparent = parent();
    oldparent->installEventFilter(this);
  }
}

bool
BeamlineView::eventFilter(QObject *target, QEvent *event){
  if ( target == oldparent && event->type() == QEvent::Resize )
    resize( parentWidget()->height() );
  return QWidget::eventFilter(target,event);
}


void
BeamlineView::resize (int h){
  QWidget::resize( woh * h, h );
}

