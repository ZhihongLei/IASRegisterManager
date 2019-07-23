#include "edit_chip_dialog.h"
#include "ui_edit_chip_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>
#include <authenticator.h>
#include <QDebug>

EditChipDialog::EditChipDialog(const QString& username, const QString& user_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditChipDialog),
    mode_(DIALOG_MODE::ADD)
{
    ui->setupUi(this);
    username2id_[username] = user_id;
    ui->comboBoxChipOwner->addItem(username);
    ui->comboBoxChipOwner->setEnabled(false);
    original_address_width_ = 0;
    original_register_width_ = 0;
    bool success = setup_ui();
    setWindowTitle("New Chip");
    if (!success)
    {
        QMessageBox::warning(this, "Edit Chip", "Unable to initialize due to database connection issue.\nPlease try again!");
        return;
    }
}

EditChipDialog::EditChipDialog(const QString& chip_id,
                        const QString& chip_name,
                        const QString& chip_owner,
                        int register_width,
                        int address_width,
                        bool msb_first,
                        QWidget *parent):
    QDialog(parent),
    ui(new Ui::EditChipDialog),
    chip_id_(chip_id),
    mode_(DIALOG_MODE::EDIT)
{
    ui->setupUi(this);
    bool success = setup_ui();
    ui->lineEditChipName->setText(chip_name);
    ui->lineEditRegWidth->setText(QString::number(register_width));
    ui->lineEditAddrWidth->setText(QString::number(address_width));
    ui->checkBoxMSBFirst->setChecked(msb_first);
    original_address_width_ = address_width;
    original_register_width_ = register_width;
    original_chip_name_ = chip_name;
    original_chip_owner_ = chip_owner;
    setWindowTitle("Edit Chip");

    QVector<QVector<QString> > items;
    success = success && DataBaseHandler::show_items_inner_join({"global_user.username", "global_user.user_id"}, {{{"global_user", "user_id"}, {"chip_designer", "user_id"}}},
                                            items,
                                            {{"chip_designer.chip_id", chip_id_}},
                                            "order by chip_designer.chip_designer_id");
    if (!success)
    {
        QMessageBox::warning(this, "Edit Chip", "Unable to initialize due to database connection issue.\nPlease try again!");
        return;
    }
    for (const auto& item : items)
    {
        ui->comboBoxChipOwner->addItem(item[0]);
        username2id_[item[0]] = item[1];
    }
    for (int i = 0; i < ui->comboBoxChipOwner->count(); i++)
        if (ui->comboBoxChipOwner->itemText(i) == chip_owner)
        {
            ui->comboBoxChipOwner->setCurrentIndex(i);
            break;
        }
}

EditChipDialog::EditChipDialog(const QString& username, const QString& user_id,
                               const QString& chip_id,
                               const QString& chip_name,
                               int register_width,
                               int address_width,
                               bool msb_first,
                               QWidget *parent):
    QDialog(parent),
    ui(new Ui::EditChipDialog),
    chip_id_(chip_id),
    mode_(DIALOG_MODE::ADD)
{
    ui->setupUi(this);
    bool success = setup_ui();
    username2id_[username] = user_id;
    ui->comboBoxChipOwner->addItem(username);
    ui->comboBoxChipOwner->setEnabled(false);
    ui->lineEditChipName->setText(chip_name);
    ui->lineEditRegWidth->setText(QString::number(register_width));
    ui->lineEditAddrWidth->setText(QString::number(address_width));
    ui->checkBoxMSBFirst->setChecked(msb_first);
    original_address_width_ = address_width;
    original_register_width_ = register_width;
    setWindowTitle("New Chip");
    if (!success)
    {
        QMessageBox::warning(this, "Edit Chip", "Unable to initialize due to database connection issue.\nPlease try again!");
        return;
    }
}

EditChipDialog::~EditChipDialog()
{
    delete ui;
}

bool EditChipDialog::setup_ui()
{
    ui->lineEditRegWidth->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*")));
    ui->lineEditAddrWidth->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*")));
    QVector<QVector<QString> > items;
    if (DataBaseHandler::show_items("def_project_role", {"project_role_id", "project_role"}, items, "", "order by project_role_id") && items.size() > 0)
    {
        project_role_id_ = items[0][0];
        project_role_ = items[0][1];
        return true;
    }
    return false;
}

QString EditChipDialog::get_chip_name() const
{
    return ui->lineEditChipName->text();
}

QString EditChipDialog::get_chip_id() const
{
    return chip_id_;
}

QString EditChipDialog::get_owner_id() const
{
    return username2id_[get_owner()];
}

QString EditChipDialog::get_owner() const
{
    return ui->comboBoxChipOwner->currentText();
}

int EditChipDialog::get_address_width() const
{
    if (ui->lineEditAddrWidth->text() == "") return -1;
    return ui->lineEditAddrWidth->text().toInt();
}

int EditChipDialog::get_register_width() const
{
    if (ui->lineEditRegWidth->text() == "") return -1;
    return ui->lineEditRegWidth->text().toInt();
}

bool EditChipDialog::msb_first() const
{
    return ui->checkBoxMSBFirst->isChecked();
}

QString EditChipDialog::get_project_role() const
{
    return project_role_;
}

QString EditChipDialog::get_project_role_id() const
{
    return project_role_id_;
}

void EditChipDialog::accept()
{
    if (sanity_check()) return QDialog::accept();
}

bool EditChipDialog::sanity_check()
{
    return check_name() && check_address_width() && check_register_width();
}

bool EditChipDialog::check_name()
{
    if (get_chip_name() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Chip name must not be empty!");
        return false;
    }

    if (get_chip_name() == original_chip_name_ && mode_ == DIALOG_MODE::EDIT) return true;
    QVector<QVector<QString> >items;
    if (!DataBaseHandler::show_items("chip_chip", {"chip_name"}, "chip_name", get_chip_name(), items))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate chip name due to database connection issue.\nPlease try again,");
        return false;
    }
    if (items.size() > 0)
    {
        QMessageBox::warning(this, windowTitle(), "Chip " + get_chip_name() + " already exists!");
        return false;
    }
    return true;
}

bool EditChipDialog::check_address_width()
{
    if (get_address_width() > 64 || get_address_width() <= 0)
    {
        QMessageBox::warning(this, windowTitle(), "Address width must range from 1 to 64!");
        return false;
    }
    if (get_address_width() < original_address_width_)
    {
        QMessageBox::warning(this, windowTitle(), "Address width must not be smaller than previous one: " + QString::number(original_address_width_) + "!");
        return false;
    }
    return true;
}

bool EditChipDialog::check_register_width()
{
    if (get_register_width() <= 0 || get_register_width() > 64)
    {
        QMessageBox::warning(this, windowTitle(), "Register width must range from 1 to 64!");
        return false;
    }
    if (get_register_width() < original_register_width_)
    {
        QMessageBox::warning(this, windowTitle(), "Register width must not be smaller than previous one: " + QString::number(original_register_width_) + "!");
        return false;
    }
    return true;
}

bool EditChipDialog::add_chip()
{
    QVector<QVector<QString> >items;
    if (DataBaseHandler::get_next_auto_increment_id("chip_chip", "chip_id", chip_id_) &&
        DataBaseHandler::insert_item("chip_chip", {"chip_id", "chip_name", "owner", "register_width", "address_width", "msb_first", "freeze"},
                                  {chip_id_,
                                  ui->lineEditChipName->text(),
                                  get_owner_id(),
                                  ui->lineEditRegWidth->text(),
                                  ui->lineEditAddrWidth->text(),
                                  ui->checkBoxMSBFirst->isChecked() ? "1" : "0", "0"}))
    {
        if (DataBaseHandler::insert_item("chip_designer", {"chip_id", "user_id", "project_role_id"}, {chip_id_, get_owner_id(), project_role_id_}))
        {
            DataBaseHandler::commit();
            return true;
        }
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, windowTitle(), "Unable to create chip.\nError message: " + DataBaseHandler::get_error_message());
    return false;
}

bool EditChipDialog::add_chip_from()
{
    QString project_role_id;
    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join({"def_db_role.full_access_to_all_projects"}, {{{"def_db_role", "db_role_id"}, {"global_user", "db_role_id"}}}, items, "global_user.user_id", get_owner_id(), "") ||
        !DataBaseHandler::show_items_inner_join({"def_project_role.read_all_blocks"},
                                        {{{"def_project_role", "project_role_id"}, {"chip_designer", "project_role_id"}}},
                                           items, {{"chip_designer.chip_id", chip_id_}, {"chip_designer.user_id", get_owner_id()}}))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to add chip due to database connection issue.\nPlease try again.");
        return false;
    }
    if (items[0][0] != "1" && (items.size() == 1 || items[1][0] != "1"))
    {
        QMessageBox::warning(this, windowTitle(),
                             "You cannot add chip from the given chip because you do not have read access to all system blocks of this chip.");
        return false;
    }
    items.clear();

    bool success = true;
    QString old_chip_id = chip_id_;
    QHash<QString, QHash<QString, QString> > in_old2news;
    QHash<QString, QString> out_old2new;

    in_old2news["owner"] = QHash<QString, QString>();
    in_old2news["owner"]["default"] = get_owner_id();
    in_old2news["chip_name"]["default"] = get_chip_name();
    in_old2news["freeze"]["default"] = "0";
    success = success && DataBaseHandler::copy_row("chip_chip", "chip_id",
                                {"chip_name", "owner", "register_width", "address_width", "msb_first", "freeze"},
                              in_old2news, out_old2new, false, "chip_id", old_chip_id);
    chip_id_ = out_old2new[old_chip_id];
    in_old2news["chip_id"] = out_old2new;
    out_old2new.clear();

    QString chip_designer_id;
    DataBaseHandler::get_next_auto_increment_id("chip_designer", "chip_designer_id", chip_designer_id);
    success = success && DataBaseHandler::insert_item("chip_designer", {"chip_id", "user_id", "project_role_id"}, {chip_id_, get_owner_id(), project_role_id_});


    in_old2news["responsible"] = QHash<QString, QString>();
    in_old2news["responsible"]["default"] = get_owner_id();
    success = success && DataBaseHandler::copy_row("block_system_block", "block_id",
                            {"block_name", "abbreviation", "chip_id", "responsible", "start_address"},
                              in_old2news, out_old2new, true, "chip_id", old_chip_id);
    in_old2news["block_id"] = out_old2new;
    out_old2new.clear();

    if (original_address_width_ != get_address_width())
    {
        success = success && DataBaseHandler::show_items("block_system_block", {"block_id", "start_address"}, "chip_id", chip_id_, items);
        for (const auto& item : items)
        {
            QString address = normalize_hex(item[1], get_address_width());
            success = success && DataBaseHandler::update_items("block_system_block", {{"block_id", item[0]}}, {{"start_address", address}});
        }
    }

    in_old2news["reg_id"] = QHash<QString, QString>();
    in_old2news["sig_id"] = QHash<QString, QString>();
    for (const QString& block_id : in_old2news["block_id"].keys())
    {
        success = success && DataBaseHandler::copy_row("block_register", "reg_id", {"reg_name", "block_id", "reg_type_id"},
                              in_old2news, out_old2new, true, "block_id", block_id);
        for (const QString& key : out_old2new.keys()) in_old2news["reg_id"][key] = out_old2new[key];
        out_old2new.clear();

        success = success && DataBaseHandler::copy_row("signal_signal", "sig_id", {"sig_name", "block_id", "width", "sig_type_id", "add_port"},
                                  in_old2news, out_old2new, false, "block_id", block_id);
        for (const QString& key : out_old2new.keys()) in_old2news["sig_id"][key] = out_old2new[key];
        out_old2new.clear();
    }


    in_old2news["reg_sig_id"] = QHash<QString, QString>();
    for (const QString& sig_id : in_old2news["sig_id"].keys())
    {
        success = success && DataBaseHandler::copy_row("signal_reg_signal", "reg_sig_id", {"sig_id", "init_value", "reg_type_id"},
                                  in_old2news, out_old2new, false, "sig_id", sig_id);
        for (const QString& key : out_old2new.keys()) in_old2news["reg_sig_id"][key] = out_old2new[key];
        out_old2new.clear();
    }


    for (const QString& reg_id : in_old2news["reg_id"].keys())
    {
        success = success && DataBaseHandler::copy_row("block_sig_reg_partition_mapping", "sig_reg_part_mapping_id",
                                {"reg_sig_id", "sig_lsb", "sig_msb", "reg_id", "reg_lsb", "reg_msb"},
                                  in_old2news, out_old2new, false, "reg_id", reg_id);
        out_old2new.clear();
    }

    in_old2news["ctrl_sig"] = in_old2news["sig_id"];
    success = success && DataBaseHandler::copy_row("chip_register_page", "page_id", {"page_name", "chip_id", "ctrl_sig", "page_count"},
                              in_old2news, out_old2new, false, "chip_id", old_chip_id);
    in_old2news["page_id"] = out_old2new;
    out_old2new.clear();

    for (const QString& page_id : in_old2news["page_id"].keys())
    {
        success = success && DataBaseHandler::copy_row("chip_register_page_content", "page_content_id", {"page_id", "reg_id"},
                                  in_old2news, out_old2new, false, "page_id", page_id);
        out_old2new.clear();
    }

    // doc
   success = success &&  DataBaseHandler::copy_row("doc_chip", "chip_doc_id", {"chip_id", "doc_type_id", "content"},
                              in_old2news, out_old2new, true, "chip_id", old_chip_id);
    for (const QString& block_id : in_old2news["block_id"].keys())
        success = success && DataBaseHandler::copy_row("doc_block", "block_doc_id", {"block_id", "doc_type_id", "content"},
                                  in_old2news, out_old2new, true, "block_id", block_id);
    for (const QString& reg_id : in_old2news["reg_id"].keys())
        success = success && DataBaseHandler::copy_row("doc_register", "register_doc_id", {"reg_id", "doc_type_id", "content"},
                                  in_old2news, out_old2new, true, "reg_id", reg_id);
    for (const QString& sig_id : in_old2news["sig_id"].keys())
        success = success && DataBaseHandler::copy_row("doc_signal", "signal_doc_id", {"sig_id", "doc_type_id", "content"},
                                  in_old2news, out_old2new, true, "sig_id", sig_id);
    return success;
}

bool EditChipDialog::edit_chip()
{
    bool success = true;
    if (get_owner() != original_chip_owner_)
    {
        success = success && DataBaseHandler::update_items("chip_designer", {{"chip_id", get_chip_id()}, {"user_id", get_owner_id()}}, {{"project_role_id", project_role_id_}});
        success = success && DataBaseHandler::update_items("chip_chip", "chip_id", chip_id_, {{"owner", get_owner_id()}});
    }
    success = success && DataBaseHandler::update_items("chip_chip", "chip_id", chip_id_,
                                    {{"chip_name", get_chip_name()}, {"register_width", QString::number(get_register_width())},
                                     {"address_width", QString::number(get_address_width())}, {"msb_first", msb_first() ? "1" : "0"}});
    if (original_address_width_ != get_address_width())
    {
        QVector<QVector<QString> > items;
        success = success && DataBaseHandler::show_items("block_system_block", {"block_id", "start_address"}, "chip_id", chip_id_, items);
        for (const auto& item : items)
        {
            QString address = normalize_hex(item[1], get_address_width());
            success = success && DataBaseHandler::update_items("block_system_block", {{"block_id", item[0]}}, {{"start_address", address}});
        }
    }

    if (success)
    {
        DataBaseHandler::commit();
        return true;
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, "Edit Chip", "Unable to edit chip\nError message: " + DataBaseHandler::get_error_message());
    return false;
}
