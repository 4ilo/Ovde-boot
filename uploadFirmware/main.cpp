#include "uploadfirmware.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    uploadFirmware w;
    w.show();

    return a.exec();
}
