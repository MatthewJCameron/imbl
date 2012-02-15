#ifndef QIMBL_H
#define QIMBL_H

#include <QtGui/QMainWindow>

namespace Ui {
class Qimbl;
}

class Qimbl : public QMainWindow
{
  Q_OBJECT

public:
  explicit Qimbl(QWidget *parent = 0);
  ~Qimbl();

private:
  Ui::Qimbl *ui;
};

#endif // QIMBL_H
