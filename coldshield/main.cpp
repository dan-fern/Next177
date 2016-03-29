#include "mountcs.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MountCS w;
    w.show();

    return a.exec();
}
