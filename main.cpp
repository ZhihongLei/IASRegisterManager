#include "register_manager.h"
#include "password.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Register_Manmager w;
    w.show();

    return a.exec();
}
