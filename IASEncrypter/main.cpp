#include "encrypter_dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EncrypterDialog w;
    w.show();
    return a.exec();
}
