#include "qimbl.h"
#include "ui_qimbl.h"

Qimbl::Qimbl(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Qimbl)
{
  ui->setupUi(this);
}

Qimbl::~Qimbl() {
  delete ui;
}
