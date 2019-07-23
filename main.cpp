#include "register_manager.h"
#include "global_variables.h"
#include <QApplication>
#include "naming_template.h"


QString MATHJAX_ROOT, HTML_TEMPLATE, HTML_TEXT_TEMPLATE, HTML_TABLE_TEMPLATE, HTML_IMAGE_TEMPLATE;
NamingTemplate GLOBAL_REGISTER_NAMING, GLOBAL_SIGNAL_NAMING;

int main(int argc, char *argv[])
{    
    QApplication app(argc, argv);
    if (!RegisterManager::initialize()) return -1;
    RegisterManager register_manager;
    return app.exec();
}
