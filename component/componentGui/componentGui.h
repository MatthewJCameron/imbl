#ifndef COMPONENTGUI_H
#define COMPONENTGUI_H

#include "component.h"
#include <QtGui>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>

class ComponentGui : public QWidget {
  Q_OBJECT;

private:
  bool deleteComponent;

protected:
  Component * comp;

public:

  ComponentGui(Component * _comp, bool _deleteComponent, QWidget * parent=0);
  ~ComponentGui();

  virtual Component * component() const;

  virtual ComponentGui * duplicate(QWidget * parent=0) = 0;

private slots:

  virtual void updateConnection(bool con) = 0;

};


class ComponentWidget : public ComponentGui {
  Q_OBJECT;
  QWidget * widget;
public:
  ComponentWidget(QWidget * wdg, QWidget * parent=0);
  ComponentWidget * duplicate(QWidget * parent=0);
public slots:
  void bringBackWidget();
private slots:
  inline void updateConnection(bool) {};
signals:
  void returned();
};



class PsswDial : public QDialog {
  Q_OBJECT;

private:

  static const QString psswd;

  QLineEdit * passwd_line;
  QDialogButtonBox * buttonBox;

  QString entered_psswd;

  PsswDial(QWidget *addition=0,  QWidget *parent = 0);

public:

  static bool ask(QWidget *parent = 0);
  static bool askAddition(QWidget * addition, QWidget *parent = 0);

};





#endif // COMPONENTGUI_H
