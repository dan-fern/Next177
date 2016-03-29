#include "mountmb.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MountMB w;
    w.show();

    return a.exec();
}
