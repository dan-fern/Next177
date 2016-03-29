#include "mountcf.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MountCF w;
    w.show();

    return a.exec();
}
