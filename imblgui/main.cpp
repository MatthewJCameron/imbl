#include <QApplication>
#include "imblgui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImblGui w;
    w.show();

    return a.exec();
}
