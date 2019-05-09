#include "zipfilesend.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ZipFileSend w;
    w.readSet();
    w.run();
    return a.exec();
}
