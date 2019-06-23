#include "register_manager.h"
#include "ui_register_manager.h"
#include "database_handler.h"
#include "create_user_dialog.h"
#include "user_management_dialog.h"
#include "change_password_dialog.h"
#include "global_variables.h"
#include "edit_chip_dialog.h"
#include "open_chip_dialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include "data_utils.h"
#include "login_dialog.h"
#include <QDebug>

RegisterManager::RegisterManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RegisterManager)
{
    ui->setupUi(this);
    //clear_db();
    //init_db();

    msb_first_ = true;

    for (QAction* action : {ui->actionUserManagement, ui->actionNewChip}) action->setEnabled(false);

    ui->splitterMain->setCollapsible(0, false);
    ui->splitterMain->setCollapsible(1, false);
    ui->splitterWorking->setCollapsible(0, false);
    ui->splitterWorking->setCollapsible(1, false);

    connect(&login_dialog_, SIGNAL(logged_in(QString)), this, SLOT(on_loggedin(QString)));
    login_dialog_.show();

    // TODO: set naming template, which is dependent of specific chips
    REGISTER_NAMING.set_naming_template(REGISTER_NAMING_TEMPLATE);
    SIGNAL_NAMING.set_naming_template(SIGNAL_NAMING_TEMPLATE);
    ui->docEditorView->set_authenticator(&authenticator_);
    ui->chipEditorView->set_authenticator(&authenticator_);
    ui->chipNavigator->set_authenticator(&authenticator_);
}


RegisterManager::~RegisterManager()
{
    delete ui;
}


void RegisterManager::on_loggedin(QString username)
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join({"global_user.user_id", "def_db_role.db_role", "def_db_role.db_role_id"},
                                    {{{"global_user", "db_role_id"}, {"def_db_role", "db_role_id"}}}, items, {{"username", username}});
    username_ = username;
    user_id_ = items[0][0];
    db_role_ = items[0][1];
    db_role_id_ = items[0][2];
    authenticator_.set_database_permissions(db_role_id_);

    ui->actionUserManagement->setEnabled(authenticator_.can_add_user() && authenticator_.can_remove_user());
    ui->actionNewChip->setEnabled(authenticator_.can_add_project());
    ui->actionChipManagement->setEnabled(authenticator_.can_add_project() && authenticator_.can_remove_project());
    setWindowTitle("IAS Register Manager - " + username);

    ui->docEditorView->login(username_, user_id_);
    ui->chipEditorView->login(username_, user_id_);
    ui->chipNavigator->login(username_, user_id_);

    login_dialog_.hide();
    OpenChipDialog open_dial(user_id_, authenticator_.can_add_project(), this);
    show();
    on_actionOpenChip_triggered();
}


void RegisterManager::open_chip()
{
    SIGNAL_NAMING.update_key("{CHIP_NAME}", chip_);
    REGISTER_NAMING.update_key("{CHIP_NAME}", chip_);
    ui->chipEditorView->open_chip(chip_, chip_id_, chip_owner_, chip_owner_id_, register_width_, address_width_, msb_first_);
    ui->docEditorView->open_chip(chip_, chip_id_, register_width_, address_width_, msb_first_);
    ui->chipNavigator->open_chip(chip_, chip_id_, msb_first_);
}

void RegisterManager::on_actionUserManagement_triggered()
{
    if (!authenticator_.can_add_user() || !authenticator_.can_remove_user())
    {
        QMessageBox::warning(this, "User Management", "You do not have access to User Manager!");
        return;
    }
    UserManagementDialog user_management(username_, this);
    user_management.exec();
}

void RegisterManager::on_actionChangePassword_triggered()
{
    ChangePasswordDialog change_password(username_, this);
    if (change_password.exec() == QDialog::Accepted) change_password.change_password();
}

void RegisterManager::on_actionLogOut_triggered()
{
    hide();
    on_actionCloseChip_triggered();
    authenticator_.clear_database_permission();
    for (QString* s : {&username_, &user_id_, &db_role_, &db_role_id_}) s->clear();
    for (QAction* action : {ui->actionUserManagement, ui->actionNewChip, ui->actionChipManagement}) action->setEnabled(false);

    login_dialog_.clear();
    login_dialog_.show();
}

void RegisterManager::on_actionNewChip_triggered()
{
    if (!authenticator_.can_add_project())
    {
        QMessageBox::warning(this, "New Chip", "You are not eligible to add projects!");
        return;
    }
    EditChipDialog new_chip(user_id_, this);
    if (new_chip.exec() == QDialog::Accepted && new_chip.add_chip())
    {
        chip_ = new_chip.get_chip_name();
        chip_id_ = new_chip.get_chip_id();
        address_width_ = new_chip.get_address_width();
        register_width_ = new_chip.get_register_width();
        chip_owner_ = username_;
        chip_owner_id_ = user_id_;
        authenticator_.set_project_permissions(new_chip.get_project_role_id());
        open_chip();
    }
}

void RegisterManager::on_actionOpenChip_triggered()
{
    OpenChipDialog open_dial(user_id_, authenticator_.can_add_project(), this);
    if (open_dial.exec() == QDialog::Accepted)
    {
        on_actionCloseChip_triggered();
        chip_ = open_dial.get_chip_name();
        chip_id_ = open_dial.get_chip_id();
        address_width_ = open_dial.get_address_width();
        register_width_ = open_dial.get_register_width();
        msb_first_ = open_dial.is_msb_first();
        chip_owner_ = open_dial.get_owner();
        chip_owner_id_ = open_dial.get_owner_id();
        authenticator_.set_project_permissions(open_dial.get_project_role_id());
        open_chip();
    }
}

void RegisterManager::on_actionCloseChip_triggered()
{
    QVector<QString*> variables = {&chip_, &chip_id_, &chip_owner_, &chip_owner_id_};
    for (QString* &v : variables) v->clear();

    address_width_ = 0;
    register_width_ = 0;
    msb_first_ = true;

    authenticator_.clear_project_permission();
    authenticator_.clear_block_permission();

    ui->chipNavigator->close_chip();
    ui->docEditorView->close_chip();
    ui->chipEditorView->close_chip();
}

void RegisterManager::on_actionChipManagement_triggered()
{
    if (!authenticator_.can_add_project() || !authenticator_.can_remove_project())
    {
        QMessageBox::warning(this, "User Management", "You do not have access to Chip Manager!");
        return;
    }
    OpenChipDialog open_dial(user_id_, chip_id_, this);
    open_dial.exec();
}

void RegisterManager::on_actionHTML_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, "Export HTML Document", "", "HTML files (*.html)");
    if (path != "")
    {
        QFile file(path);
        if( !file.open(QIODevice::WriteOnly) )
        {
          QMessageBox::warning(this, "Export HTML Document", "Unable to export document!");
          return;
        }
        QTextStream outputStream(&file);
        outputStream << ui->docEditorView->generate_html_document();
        file.close();
    }
}

void RegisterManager::on_actionDocEditorView_triggered()
{
    ui->frameDoc->setVisible(ui->actionDocEditorView->isChecked());
}

void RegisterManager::on_actionChipEditorView_triggered()
{
    ui->frameChipEditor->setVisible(ui->actionChipEditorView->isChecked());
}

void RegisterManager::on_chipNavigator_chip_clicked()
{
    ui->docEditorView->set_doc_level(LEVEL::CHIP);
    ui->docEditorView->set_block_id("");
    ui->docEditorView->display_documents();

    ui->chipEditorView->set_block_id("");
    ui->chipEditorView->display_chip_level_info();
}
void RegisterManager::on_chipNavigator_block_clicked(QString block_id)
{
    ui->docEditorView->set_doc_level(LEVEL::BLOCK);
    ui->docEditorView->set_block_id(block_id);
    ui->docEditorView->display_documents();

    ui->chipEditorView->set_block_id(block_id);
    ui->chipEditorView->display_system_level_info();
}
void RegisterManager::on_chipNavigator_register_clicked(QString block_id, QString reg_id)
{
    ui->docEditorView->set_doc_level(LEVEL::REGISTER);
    ui->docEditorView->set_register_id(reg_id);
    ui->docEditorView->display_documents();

    ui->chipEditorView->set_block_id(block_id);
    ui->chipEditorView->display_system_level_info(reg_id);
}
void RegisterManager::on_chipNavigator_signal_clicked(QString block_id, QString sig_id)
{
    ui->docEditorView->set_doc_level(LEVEL::SIGNAL);
    ui->docEditorView->set_signal_id(sig_id);
    ui->docEditorView->display_documents();

    ui->chipEditorView->set_block_id(block_id);
    ui->chipEditorView->display_system_level_info("", sig_id);
}

void RegisterManager::on_chipEditorView_block_added(QString block_id, QString block_name, QString block_abbr, QString responsible)
{
    ui->chipNavigator->add_block(block_id, block_name, block_abbr, responsible);
}
void RegisterManager::on_chipEditorView_block_removed(int row)
{
    ui->chipNavigator->remove_block(row);
}
void RegisterManager::on_chipEditorView_block_modified(int row, QString block_name, QString block_abbr, QString responsible)
{
    ui->chipNavigator->modify_block(row, block_name, block_abbr, responsible);
}
void RegisterManager::on_chipEditorView_block_order_exchanged(int from, int to)
{
    ui->chipNavigator->change_block_order(from, to);
}
void RegisterManager::on_chipEditorView_to_refresh_block()
{
    ui->chipNavigator->refresh_block();
}


void RegisterManager::init_db()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QString dbname;
    QVector<QVector<QString> > table_define;
    QString primary_key;
    QVector<QVector<QString> > foreign_keys;
    QVector<QString> unique_keys;
    QVector<QString> fields, values;

    dbhandler.create_database(gDatabase);
    // def_db_role
    dbname = "def_db_role";
    table_define = {{"db_role_id", "int", "not null auto_increment"},
                {"db_role", "varchar(20)", "not null"},
                {"add_user", "tinyint(1)", "not null"},
                {"remove_user", "tinyint(1)", "not null"},
                {"add_project", "tinyint(1)", "not null"},
                {"remove_project", "tinyint(1)", "not null"}};
    primary_key = "db_role_id";
    unique_keys = {"db_role"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);

    fields = {"db_role", "add_user", "remove_user", "add_project", "remove_project"};
    values = {"super user", "1", "1", "1", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"standard user", "0", "0", "1", "0"};
    dbhandler.insert_item(dbname, fields, values);

    // def_register_type
    dbname = "def_register_type";
    table_define = {{"reg_type_id", "int", "not null auto_increment"},
                {"reg_type", "varchar(20)", "not null"},
                {"description", "varchar(255)", "not null"}};
    primary_key = "reg_type_id";
    unique_keys = {"reg_type"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);

    fields = {"reg_type", "description"};
    values = {"R/W", "Register for standard control signals"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"RO", "Read-only register for data provided by the ASIC"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"W/SC", "Register clears after being written"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"R/SC", "Register clears after being read"};
    dbhandler.insert_item(dbname, fields, values);

    // def_signal_type
    dbname = "def_signal_type";
    table_define = {{"sig_type_id", "int", "not null auto_increment"},
                {"sig_type", "varchar(20)", "not null"},
                {"regable", "tinyint(1)", "not null"}};
    primary_key = "sig_type_id";
    unique_keys = {"sig_type"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);
    fields = {"sig_type", "regable"};
    values = {"Control Signal", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"Info Signal", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"TOP_A2A", "0"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"TOP_A2D", "0"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"TOP_D2A", "0"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"TOP_D2D", "0"};
    dbhandler.insert_item(dbname, fields, values);


    // def_sig_reg_type_mapping
    dbname = "def_sig_reg_type_mapping";
    table_define = {{"mapping_id", "int", "not null auto_increment"},
                {"sig_type_id", "int", "not null"},
                {"reg_type_id", "int", "not null"}};
    primary_key = "mapping_id";
    foreign_keys = {{"sig_type_id", "def_signal_type", "sig_type_id"},
                    {"reg_type_id", "def_register_type", "reg_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);
    fields = {"sig_type_id", "reg_type_id"};
    dbhandler.insert_item(dbname, fields, {"1", "1"});
    dbhandler.insert_item(dbname, fields, {"2", "2"});
    dbhandler.insert_item(dbname, fields, {"1", "3"});
    dbhandler.insert_item(dbname, fields, {"2", "4"});

    // def_project_role
    dbname = "def_project_role";
    table_define = {{"project_role_id", "int", "not null auto_increment"},
                {"project_role", "varchar(20)", "not null"},
                {"add_block", "tinyint(1)", "not null"},
                {"remove_his_block", "tinyint(1)", "not null"},
                {"read_all_blocks", "tinyint(1)", "not null"},
                {"compile_project", "tinyint(1)", "not null"},
                {"add_chip_designer", "tinyint(1)", "not null"},
                {"remove_chip_designer", "tinyint(1)", "not null"},
                {"full_access_to_all_blocks", "tinyint(1)", "not null"}};
    primary_key = "project_role_id";
    unique_keys = {"project_role"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);

    fields = {"project_role", "add_block", "remove_his_block", "read_all_blocks", "compile_project", "add_chip_designer", "remove_chip_designer", "full_access_to_all_blocks"};
    values = {"admin", "1", "1", "1", "1", "1", "1", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"standard designer", "1", "1", "1", "1", "0", "0", "0"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"limited designer", "0", "0", "0", "0", "0", "0", "0"};
    dbhandler.insert_item(dbname, fields, values);

    // def_doc_type
    dbname = "def_doc_type";
    table_define = {{"doc_type_id", "int", "not null auto_increment"},
                {"doc_type", "varchar(20)", "not null"}};
    primary_key = "doc_type_id";
    unique_keys = {"doc_type"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);
    dbhandler.insert_item(dbname, {"doc_type"}, {"Text"});
    dbhandler.insert_item(dbname, {"doc_type"}, {"Image"});
    dbhandler.insert_item(dbname, {"doc_type"}, {"Table"});


    // global_user
    dbname = "global_user";
    table_define = {{"user_id", "int", "not null auto_increment"},
                {"username", "varchar(256)", "not null"},
                {"password", "varchar(256)", "not null"},
                {"db_role_id", "int", "not null"}};
    primary_key = "user_id";
    foreign_keys = {{"db_role_id", "def_db_role", "db_role_id"}};
    unique_keys = {"username"};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, &unique_keys);

    dbname = "chip_chip";
    table_define = {{"chip_id", "int", "not null auto_increment"},
                {"chip_name", "varchar(256)", "not null"},
                {"owner", "int", "not null"},
                {"register_width", "int", "not null"},
                {"address_width", "int", "not null"},
                {"msb_first", "tinyint(1)", "not null"}};
    primary_key = "chip_id";
    foreign_keys = {{"owner", "global_user", "user_id"}};
    unique_keys = {"chip_name"};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, &unique_keys);

    dbname = "chip_designer";
    table_define = {{"chip_designer_id", "int", "not null auto_increment"},
                {"chip_id", "int", "not null"},
                {"user_id", "int", "not null"},
                {"project_role_id", "int", "not null"}};
    primary_key = "chip_designer_id";
    foreign_keys = {{"chip_id", "chip_chip", "chip_id"},
                    {"user_id", "global_user", "user_id"},
                    {"project_role_id", "def_project_role", "project_role_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "block_system_block";
    table_define = {{"block_id", "int", "not null auto_increment"},
                {"block_name", "varchar(256)", "not null"},
                {"abbreviation", "varchar(32)", "not null"},
                {"chip_id", "int", "not null"},
                {"responsible", "int", "not null"},
                {"start_address", "varchar(256)"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "block_id";
    foreign_keys = {{"chip_id", "chip_chip", "chip_id"},
                    {"responsible", "global_user", "user_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "signal_signal";
    table_define = {{"sig_id", "int", "not null auto_increment"},
                {"sig_name", "varchar(256)", "not null"},
                {"block_id", "int", "not null"},
                {"width", "int", "not null"},
                {"sig_type_id", "int", "not null"},
                {"add_port", "tinyint(1)", "not null"}};
    primary_key = "sig_id";
    foreign_keys = {{"block_id", "block_system_block", "block_id"},
                    {"sig_type_id", "def_signal_type", "sig_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "chip_register_page";
    table_define = {{"page_id", "int", "not null auto_increment"},
                {"page_name", "varchar(256)", "not null"},
                {"chip_id", "int", "not null"},
                {"ctrl_sig", "int", "not null"},
                {"count", "int", "not null"}};
    primary_key = "page_id";
    foreign_keys = {{"chip_id", "chip_chip", "chip_id"},
                    {"ctrl_sig", "signal_signal", "sig_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "block_register";
    table_define = {{"reg_id", "int", "not null auto_increment"},
                {"reg_name", "varchar(256)", "not null"},
                {"block_id", "int", "not null"},
                {"reg_type_id", "int", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"} };
    primary_key = "reg_id";
    foreign_keys = {{"block_id", "block_system_block", "block_id"},
                    {"reg_type_id", "def_register_type", "reg_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "chip_register_page_content";
    table_define = {{"page_content_id", "int", "not null auto_increment"},
                {"page_id", "int", "not null"},
                {"reg_id", "int", "not null"}};
    primary_key = "page_content_id";
    foreign_keys = {{"page_id", "chip_register_page", "page_id"},
                    {"reg_id", "block_register", "reg_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "signal_reg_signal";
    table_define = {{"reg_sig_id", "int", "not null auto_increment"},
                {"sig_id", "int", "not null"},
                {"init_value", "varchar(256)", "not null"},
                {"reg_type_id", "int", "not null"}};
    primary_key = "reg_sig_id";
    foreign_keys = {{"sig_id", "signal_signal", "sig_id"},
                    {"reg_type_id", "def_register_type", "reg_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "block_sig_reg_partition_mapping";
    table_define = {{"sig_reg_part_mapping_id", "int", "not null auto_increment"},
                {"reg_sig_id", "int", "not null"},
                {"sig_lsb", "int", "not null"},
                {"sig_msb", "int", "not null"},
                {"reg_id", "int", "not null"},
                {"reg_lsb", "int", "not null"},
                {"reg_msb", "int", "not null"}};
    primary_key = "sig_reg_part_mapping_id";
    foreign_keys = {{"reg_sig_id", "signal_reg_signal", "reg_sig_id"},
                    {"reg_id", "block_register", "reg_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "doc_chip";
    table_define = {{"chip_doc_id", "int", "not null auto_increment"},
                {"chip_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "chip_doc_id";
    foreign_keys = {{"chip_id", "chip_chip", "chip_id"},
                    {"doc_type_id", "def_doc_type", "doc_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "doc_block";
    table_define = {{"block_doc_id", "int", "not null auto_increment"},
                {"block_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "block_doc_id";
    foreign_keys = {{"block_id", "block_system_block", "block_id"},
                    {"doc_type_id", "def_doc_type", "doc_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "doc_register";
    table_define = {{"register_doc_id", "int", "not null auto_increment"},
                {"reg_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "register_doc_id";
    foreign_keys = {{"reg_id", "block_register", "reg_id"},
                    {"doc_type_id", "def_doc_type", "doc_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "doc_signal";
    table_define = {{"signal_doc_id", "int", "not null auto_increment"},
                {"sig_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "signal_doc_id";
    foreign_keys = {{"sig_id", "signal_signal", "sig_id"},
                    {"doc_type_id", "def_doc_type", "doc_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    // add admin
    fields = {"username", "password", "db_role_id"};
    values = {"admin", "admin", "1"};
    dbhandler.insert_item("global_user", fields, values);

}

void RegisterManager::clear_db()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    dbhandler.delete_database(gDatabase);
}
