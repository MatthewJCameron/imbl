#include <QApplication>
#include "qimbl.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  Qimbl w;
  w.show();
  
  return a.exec();
}
