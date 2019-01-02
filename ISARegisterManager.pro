#-------------------------------------------------
#
# Project created by QtCreator 2018-12-11T20:19:32
#
#-------------------------------------------------

QT       += core gui widgets

INCLUDEPATH += /usr/local/mysql-connector-c++/include/jdbc
INCLUDEPATH += /usr/local/mysql-connector-c++/include/mysqlx
INCLUDEPATH += /usr/local/include

LIBS += -L/usr/local/mysql-connector-c++/lib64 -lmysqlcppconn

TARGET = ISARegisterManager
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
        main.cpp \
        register_manager.cpp \
        database_handler.cpp \
        password.cpp
        password.cpp
        database_handler.cpp

HEADERS += \
        register_manager.h \
        password.h \
        database_handler.h
        password.h
        database_handler.h

FORMS += \
        register_manager.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
