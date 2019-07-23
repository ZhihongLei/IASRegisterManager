#-------------------------------------------------
#
# Project created by QtCreator 2019-07-23T17:20:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += /usr/local/mysql-connector-c++/include/jdbc
INCLUDEPATH += /usr/local/mysql-connector-c++/include/mysqlx
INCLUDEPATH += /usr/local/include

LIBS += -L/usr/local/mysql-connector-c++/lib64 -lmysqlcppconn

TARGET = IASDatabaseInitializer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        database_initializer_dialog.cpp \
        login_dialog.cpp \
        ../database_handler.cpp \
        main.cpp

HEADERS += \
    database_initializer_dialog.h \
    login_dialog.h \
    ../database_handler.h

FORMS += \
    database_initializer_dialog.ui \
    login_dialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
