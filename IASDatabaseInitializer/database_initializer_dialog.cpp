#include "database_initializer_dialog.h"
#include "ui_database_initializer_dialog.h"
#include "database_handler.h"
#include <QMessageBox>

DatabaseInitializerDialog::DatabaseInitializerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseInitializerDialog)
{
    ui->setupUi(this);
    setWindowTitle("IAS Database Initilizer");
}

DatabaseInitializerDialog::~DatabaseInitializerDialog()
{
    delete ui;
}

void DatabaseInitializerDialog::on_logged_in()
{
    QVector<QString> databases;
    DataBaseHandler::show_databases(databases);
    for (const QString& db : databases)
        ui->listWidget->addItem(db);
    show();
}

void DatabaseInitializerDialog::on_pushButtonAdd_clicked()
{
    if (ui->lineEditAdd->text() == "")
    {
        QMessageBox::warning(this, "Database Initilizer", "Database name must not be emtpy!");
        return;
    }
    if (DataBaseHandler::create_database(ui->lineEditAdd->text()))
    {
        ui->listWidget->addItem(ui->lineEditAdd->text());
        QString database = ui->lineEditAdd->text();
        ui->lineEditAdd->clear();
        if (init_database(database))
        {
            QMessageBox::information(this, "Database Initilizer", "Database successfully created!");
            return;
        }
    }
    QMessageBox::warning(this, "Database Initilizer", "Unable to initialize database.\nError message: " + DataBaseHandler::get_error_message());
}

bool DatabaseInitializerDialog::init_database(const QString& database)
{
    QString tablename;
    QVector<QVector<QString> > table_definition;
    QString primary_key;
    QVector<QString> fields, values;

    bool success = true;
    success = success && DataBaseHandler::use_database(database);

    // def_db_role
    tablename = "def_db_role";
    table_definition = {{"db_role_id", "int", "not null auto_increment"},
                {"db_role", "varchar(20)", "not null"},
                {"add_user", "tinyint(1)", "not null"},
                {"remove_user", "tinyint(1)", "not null"},
                {"add_project", "tinyint(1)", "not null"},
                {"remove_project", "tinyint(1)", "not null"},
                {"full_access_to_all_projects", "tinyint(1)", "not null"}};
    primary_key = "db_role_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"db_role"});


    fields = {"db_role", "add_user", "remove_user", "add_project", "remove_project", "full_access_to_all_projects"};
    values = {"super user", "1", "1", "1", "1", "1"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);
    values = {"standard user", "0", "0", "1", "0", "0"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);

    // def_register_type
    tablename = "def_register_type";
    table_definition = {{"reg_type_id", "int", "not null auto_increment"},
                        {"reg_type", "varchar(20)", "not null"},
                        {"readable", "tinyint(1)", "not null"},
                        {"writable", "tinyint(1)", "not null"},
                        {"clear", "tinyint(1)", "not null"},
                        {"description", "varchar(255)", "not null"}};
    primary_key = "reg_type_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"reg_type"});

    fields = {"reg_type", "readable", "writable", "clear", "description"};
    values = {"R/W", "1", "1", "0", "Register for standard control signals"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);
    values = {"RO", "1", "0", "0", "Read-only register for data provided by the ASIC"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);
//    values = {"W/SC", "0", "1", "1", "Register clears after being written"};
//    success = success && DataBaseHandler::insert_item(tablename, fields, values);
//    values = {"R/SC", "1", "0", "1", "Register clears after being read"};
//    success = success && DataBaseHandler::insert_item(tablename, fields, values);

    // def_signal_type
    tablename = "def_signal_type";
    table_definition = {{"sig_type_id", "int", "not null auto_increment"},
                {"sig_type", "varchar(20)", "not null"},
                {"regable", "tinyint(1)", "not null"}};
    primary_key = "sig_type_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"sig_type"});

    fields = {"sig_type", "regable"};
    values = {"Control Signal", "1"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);
    values = {"Info Signal", "1"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);
//    values = {"TOP_A2A", "0"};
//    success = success && DataBaseHandler::insert_item(tablename, fields, values);
//    values = {"TOP_A2D", "0"};
//    success = success && DataBaseHandler::insert_item(tablename, fields, values);
//    values = {"TOP_D2A", "0"};
//    success = success && DataBaseHandler::insert_item(tablename, fields, values);
//    values = {"TOP_D2D", "0"};
//    success = success && DataBaseHandler::insert_item(tablename, fields, values);


    // def_sig_reg_type_mapping
    tablename = "def_sig_reg_type_mapping";
    table_definition = {{"mapping_id", "int", "not null auto_increment"},
                {"sig_type_id", "int", "not null"},
                {"reg_type_id", "int", "not null"}};
    primary_key = "mapping_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "sig_type_id", "def_signal_type", "sig_type_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "reg_type_id", "def_register_type", "reg_type_id");

    fields = {"sig_type_id", "reg_type_id"};
    success = success && DataBaseHandler::insert_item(tablename, fields, {"1", "1"});
    success = success && DataBaseHandler::insert_item(tablename, fields, {"2", "2"});

    // def_project_role
    tablename = "def_project_role";
    table_definition = {{"project_role_id", "int", "not null auto_increment"},
                {"project_role", "varchar(20)", "not null"},
                {"add_block", "tinyint(1)", "not null"},
                {"remove_responsible_block", "tinyint(1)", "not null"},
                {"read_all_blocks", "tinyint(1)", "not null"},
                {"add_chip_designer", "tinyint(1)", "not null"},
                {"remove_chip_designer", "tinyint(1)", "not null"},
                {"full_access_to_all_blocks", "tinyint(1)", "not null"}};
    primary_key = "project_role_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"project_role"});

    fields = {"project_role", "add_block", "remove_responsible_block", "read_all_blocks", "add_chip_designer", "remove_chip_designer", "full_access_to_all_blocks"};
    values = {"admin", "1", "1", "1", "1", "1", "1"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);
    values = {"standard designer", "1", "1", "1", "0", "0", "0"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);
    values = {"limited designer", "0", "0", "0", "0", "0", "0"};
    success = success && DataBaseHandler::insert_item(tablename, fields, values);

    // def_doc_type
    tablename = "def_doc_type";
    table_definition = {{"doc_type_id", "int", "not null auto_increment"},
                {"doc_type", "varchar(20)", "not null"}};
    primary_key = "doc_type_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"doc_type"});

    success = success && DataBaseHandler::insert_item(tablename, {"doc_type"}, {"Text"});
    success = success && DataBaseHandler::insert_item(tablename, {"doc_type"}, {"Image"});
    success = success && DataBaseHandler::insert_item(tablename, {"doc_type"}, {"Table"});


    // global_user
    tablename = "global_user";
    table_definition = {{"user_id", "int", "not null auto_increment"},
                {"username", "varchar(256)", "not null"},
                {"password", "varchar(256)", "not null"},
                {"db_role_id", "int", "not null"}};
    primary_key = "user_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"username"});
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "db_role_id", "def_db_role", "db_role_id");

    tablename = "chip_chip";
    table_definition = {{"chip_id", "int", "not null auto_increment"},
                {"chip_name", "varchar(256)", "not null"},
                {"owner", "int", "not null"},
                {"register_width", "int", "not null"},
                {"address_width", "int", "not null"},
                {"msb_first", "tinyint(1)", "not null"},
                {"freeze", "tinyint(1)", "not null"}};
    primary_key = "chip_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"chip_name"});
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "owner", "global_user", "user_id");

    tablename = "chip_designer";
    table_definition = {{"chip_designer_id", "int", "not null auto_increment"},
                {"chip_id", "int", "not null"},
                {"user_id", "int", "not null"},
                {"project_role_id", "int", "not null"}};
    primary_key = "chip_designer_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "chip_id", "chip_chip", "chip_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "user_id", "global_user", "user_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "project_role_id", "def_project_role", "project_role_id");
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"chip_id", "user_id"});

    tablename = "block_system_block";
    table_definition = {{"block_id", "int", "not null auto_increment"},
                {"block_name", "varchar(256)", "not null"},
                {"abbreviation", "varchar(32)", "not null"},
                {"chip_id", "int", "not null"},
                {"responsible", "int", "not null"},
                {"start_address", "varchar(256)"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "block_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "chip_id", "chip_chip", "chip_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "responsible", "global_user", "user_id");
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"chip_id", "block_name"});
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"chip_id", "abbreviation"});


    tablename = "signal_signal";
    table_definition = {{"sig_id", "int", "not null auto_increment"},
                {"sig_name", "varchar(256)", "not null"},
                {"block_id", "int", "not null"},
                {"width", "int", "not null"},
                {"sig_type_id", "int", "not null"},
                {"add_port", "tinyint(1)", "not null"}};
    primary_key = "sig_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "block_id", "block_system_block", "block_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "sig_type_id", "def_signal_type", "sig_type_id");
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"block_id", "sig_name"});


    tablename = "chip_register_page";
    table_definition = {{"page_id", "int", "not null auto_increment"},
                {"page_name", "varchar(256)", "not null"},
                {"chip_id", "int", "not null"},
                {"ctrl_sig", "int", "not null"},
                {"count", "int", "not null"}};
    primary_key = "page_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "chip_id", "chip_chip", "chip_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "ctrl_sig", "signal_signal", "sig_id");
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"chip_id", "page_name"});
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"ctrl_sig"});

    tablename = "block_register";
    table_definition = {{"reg_id", "int", "not null auto_increment"},
                {"reg_name", "varchar(256)", "not null"},
                {"block_id", "int", "not null"},
                {"reg_type_id", "int", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"} };
    primary_key = "reg_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "block_id", "block_system_block", "block_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "reg_type_id", "def_register_type", "reg_type_id");
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"block_id", "reg_name"});


    tablename = "chip_register_page_content";
    table_definition = {{"page_content_id", "int", "not null auto_increment"},
                {"page_id", "int", "not null"},
                {"reg_id", "int", "not null"}};
    primary_key = "page_content_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "page_id", "chip_register_page", "page_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "reg_id", "block_register", "reg_id");
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"reg_id"});

    tablename = "signal_reg_signal";
    table_definition = {{"reg_sig_id", "int", "not null auto_increment"},
                {"sig_id", "int", "not null"},
                {"init_value", "varchar(256)", "not null"},
                {"reg_type_id", "int", "not null"}};
    primary_key = "reg_sig_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "sig_id", "signal_signal", "sig_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "reg_type_id", "def_register_type", "reg_type_id");
    success = success && DataBaseHandler::add_unique_key_constraint(tablename, {"sig_id"});


    tablename = "block_sig_reg_partition_mapping";
    table_definition = {{"sig_reg_part_mapping_id", "int", "not null auto_increment"},
                {"reg_sig_id", "int", "not null"},
                {"sig_lsb", "int", "not null"},
                {"sig_msb", "int", "not null"},
                {"reg_id", "int", "not null"},
                {"reg_lsb", "int", "not null"},
                {"reg_msb", "int", "not null"}};
    primary_key = "sig_reg_part_mapping_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "reg_sig_id", "signal_reg_signal", "reg_sig_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "reg_id", "block_register", "reg_id");


    tablename = "doc_chip";
    table_definition = {{"chip_doc_id", "int", "not null auto_increment"},
                {"chip_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "chip_doc_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "chip_id", "chip_chip", "chip_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "doc_type_id", "def_doc_type", "doc_type_id");

    tablename = "doc_block";
    table_definition = {{"block_doc_id", "int", "not null auto_increment"},
                {"block_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "block_doc_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "block_id", "block_system_block", "block_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "doc_type_id", "def_doc_type", "doc_type_id");

    tablename = "doc_register";
    table_definition = {{"register_doc_id", "int", "not null auto_increment"},
                {"reg_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "register_doc_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "reg_id", "block_register", "reg_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "doc_type_id", "def_doc_type", "doc_type_id");

    tablename = "doc_signal";
    table_definition = {{"signal_doc_id", "int", "not null auto_increment"},
                {"sig_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "signal_doc_id";
    success = success && DataBaseHandler::create_table(tablename, table_definition, primary_key);
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "sig_id", "signal_signal", "sig_id");
    success = success && DataBaseHandler::add_foreign_key_constraint(tablename, "doc_type_id", "def_doc_type", "doc_type_id");


    // add admin
    fields = {"username", "password", "db_role_id"};
    values = {"admin", "admin", "1"};
    success = success && DataBaseHandler::insert_item("global_user", fields, values);

    if (success) DataBaseHandler::commit();
    else DataBaseHandler::rollback();

    return success;
}
