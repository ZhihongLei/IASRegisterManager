#-------------------------------------------------
#
# Project created by QtCreator 2018-12-11T20:19:32
#
#-------------------------------------------------

QT       += core gui widgets webenginewidgets webengine

INCLUDEPATH += /usr/local/mysql-connector-c++/include/jdbc
INCLUDEPATH += /usr/local/mysql-connector-c++/include/mysqlx
INCLUDEPATH += /usr/local/include

LIBS += -L/usr/local/mysql-connector-c++/lib64 -lmysqlcppconn

TARGET = IASRegisterManager
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
    authenticator.cpp \
    chip_editor_view.cpp \
    chip_navigator.cpp \
    database_utils.cpp \
    document_editor_view.cpp \
    document_generation_dialog.cpp \
    document_generator.cpp \
    edit_chip_designer_dialog.cpp \
    edit_document_dialog.cpp \
    edit_naming_template_dialog.cpp \
    edit_register_page_dialog.cpp \
    line_edit.cpp \
    main.cpp \
    naming_template.cpp \
    naming_template_dialog.cpp \
    plain_text_edit.cpp \
    qaesencryption.cpp \
    register_manager.cpp \
    database_handler.cpp \
    login_dialog.cpp \
    edit_signal_dialog.cpp \
    create_user_dialog.cpp \
    change_password_dialog.cpp \
    edit_system_block_dialog.cpp \
    edit_register_dialog.cpp \
    resources_base_dir_dialog.cpp \
    spi_generation_dialog.cpp \
    user_management_dialog.cpp \
    edit_chip_dialog.cpp \
    open_chip_dialog.cpp \
    edit_signal_partition_dialog.cpp \
    edit_signal_partition_logic.cpp \
    data_utils.cpp \
    vhdl_generator.cpp

HEADERS += \
    authenticator.h \
    chip_editor_view.h \
    chip_navigator.h \
    database_utils.h \
    document_editor_view.h \
    document_generation_dialog.h \
    document_generator.h \
    edit_chip_designer_dialog.h \
    edit_document_dialog.h \
    edit_naming_template_dialog.h \
    edit_register_page_dialog.h \
    line_edit.h \
    naming_template.h \
    naming_template_dialog.h \
    plain_text_edit.h \
    qaesencryption.h \
    register_manager.h \
    database_handler.h \
    login_dialog.h \
    global_variables.h \
    edit_signal_dialog.h \
    create_user_dialog.h \
    change_password_dialog.h \
    edit_system_block_dialog.h \
    edit_register_dialog.h \
    resources_base_dir_dialog.h \
    spi_generation_dialog.h \
    user_management_dialog.h \
    edit_chip_dialog.h \
    open_chip_dialog.h \
    edit_signal_partition_dialog.h \
    edit_signal_partition_logic.h \
    data_utils.h \
    vhdl_generator.h

FORMS += \
    chip_editor_view.ui \
    chip_navigator.ui \
    document_editor_view.ui \
    document_generation_dialog.ui \
    edit_chip_designer_dialog.ui \
    edit_document_dialog.ui \
    edit_naming_template_dialog.ui \
    edit_register_page_dialog.ui \
    naming_template_dialog.ui \
    register_manager.ui \
    login_dialog.ui \
    edit_signal_dialog.ui \
    create_user_dialog.ui \
    change_password_dialog.ui \
    edit_system_block_dialog.ui \
    edit_register_dialog.ui \
    resources_base_dir_dialog.ui \
    spi_generation_dialog.ui \
    user_management_dialog.ui \
    edit_chip_dialog.ui \
    open_chip_dialog.ui \
    edit_signal_partition_dialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
