#include "register_manager.h"
#include "ui_register_manager.h"
#include "database_handler.h"
#include "user_management_dialog.h"
#include "change_password_dialog.h"
#include "global_variables.h"
#include "edit_chip_dialog.h"
#include "open_chip_dialog.h"
#include "data_utils.h"
#include "authenticator.h"
#include "login_dialog.h"
#include "naming_template_dialog.h"
#include "resources_base_dir_dialog.h"
#include "document_generator.h"
#include "document_generation_dialog.h"
#include "spi_generation_dialog.h"
#include "qaesencryption.h"
#include <QDebug>
#include <QSettings>
#include <QCompleter>
#include <QDir>
#include <QMessageBox>
#include <QDate>


RegisterManager::RegisterManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RegisterManager),
    login_dialog_(new LoginDialog()),
    authenticator_(new Authenticator()),
    completer_(nullptr)
{
    ui->setupUi(this);
    ui->actionNewChip->setShortcut(QKeySequence::New);
    ui->actionOpenChip->setShortcut(QKeySequence::Open);
    ui->actionCloseChip->setShortcut(QKeySequence::Close);

    ui->actionDocEditor->setShortcut(QKeySequence("Ctrl+D").toString(QKeySequence::NativeText));
    ui->actionChipEditorView->setShortcut(QKeySequence("Ctrl+E").toString(QKeySequence::NativeText));
    ui->actionDocPreview->setShortcut(QKeySequence("Ctrl+P").toString(QKeySequence::NativeText));

    msb_first_ = true;
    chip_opened_ = false;

    for (QAction* action : {ui->actionUserManagement, ui->actionNewChip}) action->setEnabled(false);

    ui->splitterMain->setCollapsible(0, false);
    ui->splitterMain->setCollapsible(1, false);
    ui->splitterWorking->setCollapsible(0, false);
    ui->splitterWorking->setCollapsible(1, false);

    connect(login_dialog_, SIGNAL(logged_in(QString)), this, SLOT(on_loggedin(QString)));
    login_dialog_->show();

    ui->docEditorView->set_authenticator(authenticator_);
    ui->chipEditorView->set_authenticator(authenticator_);
    ui->chipNavigator->set_authenticator(authenticator_);

    ui->frameDoc->setVisible(false);
    ui->actionDocEditor->setChecked(false);
    ui->actionDocPreview->setChecked(false);

    for (QAction* a : {ui->actionNaming, ui->actionResourcesBaseDir, ui->actionDocument, ui->actionSPISourceCode, ui->actionCloseChip, ui->actionFreezeChip})
        a->setEnabled(false);
    ui->actionFreezeChip->setText("Freeze");

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("user");
    bool save_password = settings.value("save_password").toBool();
    login_dialog_->set_save_password(save_password);
    login_dialog_->set_username(settings.value("username").toString());
    if (save_password)
    {
        QString key = settings.value("key").toString(),
                encrypted_password = settings.value("encrypted_password").toString();
        QString password = QAESEncryption::decode(encrypted_password, key);
        login_dialog_->set_password(password);
    }
    settings.endGroup();
}


RegisterManager::~RegisterManager()
{
    hide();
    if (authenticator_) delete authenticator_;
    if (login_dialog_) delete  login_dialog_;
    if (completer_) delete  completer_;
    DataBaseHandler::close();

    if (username_ == "") return;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("ui");
    settings.setValue("fullscreen", isFullScreen());
    settings.setValue("mainwindow_width", width());
    settings.setValue("mainwindow_height", height());
    settings.setValue("navigator_width", ui->splitterMain->sizes()[0]);
    settings.setValue("chip_editor_width", ui->splitterWorking->sizes()[0]);
    settings.setValue("doc_editor_width", ui->splitterWorking->sizes()[1]);
    settings.setValue("document_preview", ui->actionDocPreview->isChecked());
    settings.setValue("document_editor_view", ui->actionDocEditor->isChecked());
    settings.setValue("document_preview", ui->actionDocPreview->isChecked());
    settings.setValue("document_editor_view", ui->actionDocEditor->isChecked());
    settings.setValue("chip_editor_view", ui->actionChipEditorView->isChecked());

    delete ui;
}

bool RegisterManager::initialize()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "global_settings");
    settings.beginGroup("database");
    QString host = settings.value("host").toString(),
            port = settings.value("port").toString(),
            database = settings.value("database").toString(),
            username = settings.value("username").toString(),
            key = settings.value("key").toString(),
            encoded = settings.value("encrypted_password").toString(),
            password = QAESEncryption::decode(encoded, key);
    settings.endGroup();
    if (port != "") host = host + ":" + port;
    if (!DataBaseHandler::initialize(host, database, username, password))
    {
        QMessageBox::warning(nullptr, "Register Manager", "Unable to login to database.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }

    settings.beginGroup("mathjax");
    MATHJAX_ROOT = settings.value("path").toString();
    if (MATHJAX_ROOT == "") MATHJAX_ROOT = QDir::current().absoluteFilePath("MathJax");
    settings.endGroup();

    settings.beginGroup("html_templates");
    QVector<QString*> templates = {&HTML_TEMPLATE, &HTML_TEXT_TEMPLATE, &HTML_TABLE_TEMPLATE, &HTML_IMAGE_TEMPLATE};
    QVector<QString> keys = {"HTML_TEMPLATE", "HTML_TEXT_TEMPLATE", "HTML_TABLE_TEMPLATE", "HTML_IMAGE_TEMPLATE"};
    QVector<QString> default_templates = {DEFAULT_HTML_TEMPLATE, DEFAULT_HTML_TEXT_TEMPLATE,
                                          DEFAULT_HTML_IMAGE_TEMPLATE, DEFAULT_HTML_TABLE_TEMPLATE};

    for (int i = 0; i < templates.size(); i++)
    {
        QString html_template_path = settings.value(keys[i]).toString();
        if (html_template_path != "")
        {
            QFile file(html_template_path);
            if (file.open(QIODevice::ReadOnly))
            {
                QTextStream stream(&file);
                *templates[i] = stream.readAll();
                continue;
            }
        }
        *templates[i] = default_templates[i];
    }
    settings.endGroup();

    settings.beginGroup("log");
    LOG_PATH = settings.value("path").toString();
    if (LOG_PATH == "")
        QMessageBox::warning(nullptr, "Logger", "Log file path is not specified.\nLogging will not work properly.");
    else {
        if (QDir::isRelativePath(LOG_PATH)) LOG_PATH = QDir::home().absoluteFilePath(LOG_PATH);
        qInstallMessageHandler(RegisterManager::LogMessageHandler);
    }
    settings.endGroup();
    return true;
}

void RegisterManager::on_loggedin(QString username)
{
    qInfo().noquote() << QString("Logged in as %1.").arg(username);
    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join({"global_user.user_id", "def_db_role.db_role", "def_db_role.db_role_id"},
                                    {{{"global_user", "db_role_id"}, {"def_db_role", "db_role_id"}}}, items, {{"username", username}}))
    {
        QMessageBox::warning(this, "Login", "Unable to login due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }

    username_ = username;
    user_id_ = items[0][0];
    db_role_ = items[0][1];
    db_role_id_ = items[0][2];
    authenticator_->set_database_permissions(db_role_id_);

    ui->actionUserManagement->setEnabled(authenticator_->can_add_user() && authenticator_->can_remove_user());
    ui->actionNewChip->setEnabled(authenticator_->can_add_project());
    ui->actionChipManagement->setEnabled(authenticator_->can_add_project() && authenticator_->can_remove_project());
    setWindowTitle("IAS Register Manager - " + username);

    ui->docEditorView->login(username_, user_id_);
    ui->chipEditorView->login(username_, user_id_);
    ui->chipNavigator->login(username_, user_id_);

    login_dialog_->hide();

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("user");
    settings.setValue("username", username);
    settings.setValue("save_password", login_dialog_->save_password());
    if (login_dialog_->save_password())
    {
        QString key = QAESEncryption::generate_key();
        QString encrypted = QAESEncryption::encode(login_dialog_->get_password(), key);
        settings.setValue("key", key);
        settings.setValue("encrypted_password", encrypted);
    }
    else {
        settings.setValue("key", "");
        settings.setValue("encrypted_password", "");
    }
    settings.endGroup();

    settings.beginGroup("ui");
    int navigator_width = settings.value("navigator_width").toInt(),
        chip_editor_width = settings.value("chip_editor_width").toInt(),
        doc_editor_width = settings.value("doc_editor_width").toInt();
    int width = settings.value("mainwindow_width").toInt();
    int height = settings.value("mainwindow_height").toInt();
    resize(width, height);
    show();

    if (settings.value("chip_editor_view").toBool() != ui->actionChipEditorView->isChecked())
    {
        ui->actionChipEditorView->setChecked(settings.value("chip_editor_view").toBool());
        on_actionChipEditorView_triggered();
    }
    if (settings.value("document_editor_view").toBool() != ui->actionDocEditor->isChecked())
    {
        ui->actionDocEditor->setChecked(settings.value("document_editor_view").toBool());
        on_actionDocEditor_triggered();
    }
    if (settings.value("document_preview").toBool() != ui->actionDocPreview->isChecked())
    {
        ui->actionDocPreview->setChecked(settings.value("document_preview").toBool());
        on_actionDocPreview_triggered();
    }
    ui->splitterMain->setSizes({navigator_width, chip_editor_width + doc_editor_width});
    ui->splitterWorking->setSizes({chip_editor_width, doc_editor_width});

    on_actionOpenChip_triggered();
    if (settings.value("fullscreen").toBool()) showFullScreen();
}


void RegisterManager::open_chip()
{
    qInfo().noquote() << QString("Chip %2 [%3] opened.").arg(chip_name_, chip_id_);
    QSettings chip_settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "chip_settings");;
    chip_settings.beginGroup(chip_id_);
    RESOURCES_BASE_DIR = chip_settings.value("resources_base_dir").toString();
    QString reg_naming_template = chip_settings.value("register_naming_template").toString();
    QString sig_naming_template = chip_settings.value("signal_naming_template").toString();
    if (reg_naming_template == "")
    {
        reg_naming_template = DEFAULT_REGISTER_NAMING_TEMPLATE;
        chip_settings.setValue("register_naming_template", reg_naming_template);
    }
    if (sig_naming_template == "")
    {
        sig_naming_template = DEFAULT_SIGNAL_NAMING_TEMPLATE;
        chip_settings.setValue("signal_naming_template", sig_naming_template);
    }

    GLOBAL_SIGNAL_NAMING.set_naming_template(sig_naming_template);
    GLOBAL_REGISTER_NAMING.set_naming_template(reg_naming_template);

    GLOBAL_SIGNAL_NAMING.update_key("${CHIP_NAME}", chip_name_);
    GLOBAL_REGISTER_NAMING.update_key("${CHIP_NAME}", chip_name_);
    ui->chipNavigator->open_chip(chip_name_, chip_id_, msb_first_);
    ui->chipEditorView->open_chip(chip_name_, chip_id_, chip_owner_, chip_owner_id_, register_width_, address_width_, msb_first_);
    ui->docEditorView->open_chip(chip_name_, chip_id_, register_width_, address_width_, msb_first_);
    if (ui->actionDocEditor->isChecked()) ui->docEditorView->display_documents();
    else if (ui->actionDocPreview->isChecked()) ui->docEditorView->display_overall_documents();
    set_completer();

    for (QAction* a : {ui->actionNaming, ui->actionResourcesBaseDir, ui->actionDocument, ui->actionSPISourceCode, ui->actionCloseChip})
        a->setEnabled(true);
    ui->actionFreezeChip->setEnabled(chip_owner_ == username_ || authenticator_->can_fully_access_all_projects());

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("recent_projects");
    int pos = 1;
    QString candidate = settings.value("project" + QString::number(pos)).toString();
    while (candidate != "" && candidate != chip_id_)
    {
        pos++;
        candidate = settings.value("project"+QString::number(pos)).toString();
    }
    for (int i = pos; i > 1; i--)
        settings.setValue("project"+QString::number(i), settings.value("project"+QString::number(i-1)).toString());
    settings.setValue("project1", chip_id_);

    chip_opened_ = true;
}

void RegisterManager::set_completer()
{
    QStringList words;
    words << chip_name_ + " ";
    QVector<QVector<QString> > items;

    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator_->can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id_}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id_}, {"block_system_block.responsible", user_id_}};
    DataBaseHandler::show_items("block_system_block", {"block_id", "block_name", "abbreviation"}, key_value_pairs, items, "order by block_id");

    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING,
                   register_naming = GLOBAL_REGISTER_NAMING;
    for (const auto& item : items)
    {
        QString block_id = item[0], block_name = item[1], block_abbr = item[2];
        words << block_name + " " << block_abbr + " ";
        signal_naming.update_key("${BLOCK_NAME}", block_name);
        signal_naming.update_key("${BLOCK_ABBR}", block_abbr);
        register_naming.update_key("${BLOCK_NAME}", block_name);
        register_naming.update_key("${BLOCK_ABBR}", block_abbr);

        QVector<QVector<QString> > reg_items;
        DataBaseHandler::show_items("block_register", {"reg_name"}, "block_id", block_id, reg_items);
        for (const auto & reg_item : reg_items)
            words << register_naming.get_extended_name(reg_item[0]) + " ";

        QVector<QVector<QString> > sig_items;
        DataBaseHandler::show_items("signal_signal", {"sig_name", "add_port"}, "block_id", block_id, sig_items);
        for (const auto & sig_item : sig_items)
            words <<  (sig_item[1] == "1" ? signal_naming.get_extended_name(sig_item[0]) : sig_item[0]) + " ";
    }

    QDir dir("completion");
    QStringList files = dir.entryList({"*.txt"}, QDir::Files);
    for (const QString& file : files)
    {
        QFile f(dir.absoluteFilePath(file));
        if (f.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&f);
            while (!stream.atEnd()) words << stream.readLine();
        }
    }

    QCompleter * c = new QCompleter(words);
    ui->docEditorView->set_completer(c);
    if (completer_) delete completer_;
    completer_ = c;
}


void RegisterManager::on_actionUserManagement_triggered()
{
    if (!authenticator_->can_add_user() || !authenticator_->can_remove_user())
    {
        QMessageBox::warning(this, "User Management", "You do not have access to User Manager!");
        return;
    }
    UserManagementDialog user_management(chip_id_, user_id_, this);
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
    authenticator_->clear_database_permission();
    for (QString* s : {&username_, &user_id_, &db_role_, &db_role_id_}) s->clear();
    for (QAction* action : {ui->actionUserManagement, ui->actionNewChip, ui->actionChipManagement}) action->setEnabled(false);
    if (!login_dialog_->save_password()) login_dialog_->clear();
    login_dialog_->show();
}

void RegisterManager::on_actionNewChip_triggered()
{
    if (!authenticator_->can_add_project())
    {
        QMessageBox::warning(this, "New Chip", "You are not eligible to create chips!");
        return;
    }
    EditChipDialog new_chip(username_, user_id_, this);
    if (new_chip.exec() == QDialog::Accepted && new_chip.add_chip())
    {
        chip_name_ = new_chip.get_chip_name();
        chip_id_ = new_chip.get_chip_id();
        address_width_ = new_chip.get_address_width();
        register_width_ = new_chip.get_register_width();
        chip_owner_ = username_;
        chip_owner_id_ = user_id_;
        if (authenticator_->can_fully_access_all_projects()) authenticator_->set_project_permissions(true);
        else authenticator_->set_project_permissions(new_chip.get_project_role_id());
        authenticator_->freeze(false);
        ui->actionFreezeChip->setText("Freeze");
        open_chip();
    }
}

void RegisterManager::on_actionNewChipFrom_triggered()
{
    if (!authenticator_->can_add_project())
    {
        QMessageBox::warning(this, "New Chip", "You are not eligible to create chips!");
        return;
    }
    OpenChipDialog open_chip_dialog(username_, user_id_, false, this);
    open_chip_dialog.setWindowTitle("Select Chip");
    if (open_chip_dialog.exec() == QDialog::Accepted)
    {
        QString chip_id = open_chip_dialog.get_chip_id(),
                chip_name = open_chip_dialog.get_chip_name();
        int address_width = open_chip_dialog.get_address_width(),
            register_width = open_chip_dialog.get_register_width();
        bool msb_first = open_chip_dialog.msb_first();
        EditChipDialog new_chip(username_, user_id_, chip_id, chip_name, register_width, address_width, msb_first, this);
        if (new_chip.exec() == QDialog::Accepted && new_chip.add_chip_from())
        {
            QMessageBox::information(this, "New Chip", "Chip successfully created from " + chip_name + "!");
            chip_name_ = new_chip.get_chip_name();
            chip_id_ = new_chip.get_chip_id();
            address_width_ = new_chip.get_address_width();
            register_width_ = new_chip.get_register_width();
            chip_owner_ = username_;
            chip_owner_id_ = user_id_;
            if (authenticator_->can_fully_access_all_projects()) authenticator_->set_project_permissions(true);
            else authenticator_->set_project_permissions(new_chip.get_project_role_id());
            authenticator_->freeze(false);
            ui->actionFreezeChip->setText("Freeze");
            open_chip();
        }
    }
}

void RegisterManager::on_actionOpenChip_triggered()
{
    OpenChipDialog open_chip_dialog(username_, user_id_, authenticator_->can_add_project(), this);
    if (open_chip_dialog.exec() == QDialog::Accepted)
    {
        on_actionCloseChip_triggered();
        chip_name_ = open_chip_dialog.get_chip_name();
        chip_id_ = open_chip_dialog.get_chip_id();
        address_width_ = open_chip_dialog.get_address_width();
        register_width_ = open_chip_dialog.get_register_width();
        msb_first_ = open_chip_dialog.msb_first();
        chip_owner_ = open_chip_dialog.get_owner();
        chip_owner_id_ = open_chip_dialog.get_owner_id();
        if (authenticator_->can_fully_access_all_projects()) authenticator_->set_project_permissions(true);
        else authenticator_->set_project_permissions(open_chip_dialog.get_project_role_id());
        authenticator_->freeze(open_chip_dialog.frozen());
        ui->actionFreezeChip->setText(open_chip_dialog.frozen() ? "Unfreeze" : "Freeze");
        open_chip();
    }
}

void RegisterManager::on_actionCloseChip_triggered()
{
    QVector<QString*> variables = {&chip_name_, &chip_id_, &chip_owner_, &chip_owner_id_};
    for (QString* &v : variables) v->clear();

    address_width_ = 0;
    register_width_ = 0;
    msb_first_ = true;
    chip_opened_ = false;

    authenticator_->clear_project_permission();
    authenticator_->clear_block_permission();
    authenticator_->freeze(false);

    ui->chipNavigator->close_chip();
    ui->docEditorView->close_chip();
    ui->chipEditorView->close_chip();

    for (QAction* a : {ui->actionNaming, ui->actionResourcesBaseDir, ui->actionDocument, ui->actionSPISourceCode, ui->actionCloseChip, ui->actionFreezeChip})
        a->setEnabled(false);
    ui->actionFreezeChip->setText("Freeze");
    RESOURCES_BASE_DIR = "";
}

void RegisterManager::on_actionNaming_triggered()
{
    NamingTemplateDialog naming(chip_id_, this);
    if (naming.exec() == QDialog::Accepted && (GLOBAL_SIGNAL_NAMING.get_naming_template() != naming.get_signal_naming() ||
                                               GLOBAL_REGISTER_NAMING.get_naming_template() != naming.get_register_naming()))
    {
        QSettings chip_settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "chip_settings");;
        chip_settings.beginGroup(chip_id_);
        GLOBAL_SIGNAL_NAMING.set_naming_template(naming.get_signal_naming());
        GLOBAL_REGISTER_NAMING.set_naming_template(naming.get_register_naming());
        chip_settings.setValue("signal_naming_template", naming.get_signal_naming());
        chip_settings.setValue("register_naming_template", naming.get_register_naming());
        open_chip();
    }
}

void RegisterManager::on_actionResourcesBaseDir_triggered()
{
    ResourcesBaseDirDialog dialog(RESOURCES_BASE_DIR, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QSettings chip_settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "chip_settings");;
        chip_settings.beginGroup(chip_id_);
        chip_settings.setValue("resources_base_dir", dialog.get_base_dir());
        RESOURCES_BASE_DIR = dialog.get_base_dir();
    }
}

void RegisterManager::on_actionFreezeChip_triggered()
{
    if (DataBaseHandler::update_items("chip_chip", "chip_id", chip_id_, {{"freeze", ui->actionFreezeChip->text() == "Freeze" ? "1" : "0"}}))
    {
        DataBaseHandler::commit();
        authenticator_->freeze(ui->actionFreezeChip->text() == "Freeze");
        ui->actionFreezeChip->setText(ui->actionFreezeChip->text() == "Freeze" ? "Unfreeze" : "Freeze");
        ui->chipNavigator->open_chip(chip_name_, chip_id_, msb_first_);
        ui->chipEditorView->open_chip(chip_name_, chip_id_, chip_owner_, chip_owner_id_, register_width_, address_width_, msb_first_);
        ui->docEditorView->open_chip(chip_name_, chip_id_, register_width_, address_width_, msb_first_);
        if (ui->actionDocEditor->isChecked()) ui->docEditorView->display_documents();
        else if (ui->actionDocPreview->isChecked()) ui->docEditorView->display_overall_documents();
    }
    else
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, ui->actionFreezeChip->text() + " Chip",
                             "Unable to " + ui->actionFreezeChip->text() + " chip.\nError message: " + DataBaseHandler::get_error_message());
    }
}

void RegisterManager::on_actionChipManagement_triggered()
{
    if (!authenticator_->can_add_project() || !authenticator_->can_remove_project())
    {
        QMessageBox::warning(this, "User Management", "You do not have access to Chip Manager!");
        return;
    }
    OpenChipDialog open_chip_dialog(username_, user_id_, chip_id_, this);
    open_chip_dialog.exec();
}

void RegisterManager::on_actionDocument_triggered()
{
    DocumentGenerationDialog generator(chip_id_, chip_name_, register_width_, address_width_, msb_first_, authenticator_, user_id_, this);
    QSettings chip_settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "chip_settings");;
    chip_settings.beginGroup(chip_id_);
    if (generator.exec() == QDialog::Accepted)
    {
        if (GLOBAL_SIGNAL_NAMING.get_naming_template() != generator.get_signal_naming() || GLOBAL_REGISTER_NAMING.get_naming_template() != generator.get_register_naming())
        {
            GLOBAL_SIGNAL_NAMING.set_naming_template(generator.get_signal_naming());
            GLOBAL_REGISTER_NAMING.set_naming_template(generator.get_register_naming());
            chip_settings.setValue("signal_naming_template", generator.get_signal_naming());
            chip_settings.setValue("register_naming_template", generator.get_register_naming());
            open_chip();
        }
        chip_settings.setValue("image_caption_position", generator.get_image_caption_position());
        chip_settings.setValue("table_caption_position", generator.get_table_caption_position());
        chip_settings.setValue("show_paged_register", generator.get_show_paged_register());
        generator.generate_document();
    }
}

void RegisterManager::on_actionSPISourceCode_triggered()
{
    SPIGenerationDialog generator(chip_id_, chip_name_, register_width_, address_width_, this);
    if (generator.exec() == QDialog::Accepted && generator.generate_spi_interface())
    {

    }
}

void RegisterManager::on_actionDocEditor_triggered()
{
    if (ui->actionDocPreview->isChecked())
    {
        ui->actionDocPreview->setChecked(false);
        ui->frameNavigator->setVisible(true);
    }
    ui->frameDoc->setVisible(ui->actionDocEditor->isChecked());
    if (ui->actionDocEditor->isChecked()) ui->labelDocEditor->setText("Document Editor");
    if (ui->frameDoc->isVisible())
    {
        ui->docEditorView->set_view(DocumentEditorView::EDITOR_VIEW);
        if (chip_opened_) ui->docEditorView->display_documents();
    }
}

void RegisterManager::on_actionDocPreview_triggered()
{
    if (ui->actionDocEditor->isChecked())
    {
        ui->actionDocEditor->setChecked(false);
    }
    ui->frameDoc->setVisible(ui->actionDocPreview->isChecked());
    if (ui->actionDocPreview->isChecked()) ui->labelDocEditor->setText("Document Preview");
    if (ui->frameDoc->isVisible())
    {
        ui->docEditorView->set_view(DocumentEditorView::PREVIEW);
        if (chip_opened_) ui->docEditorView->display_overall_documents();
    }
}

void RegisterManager::on_actionChipEditorView_triggered()
{
    ui->frameChipEditor->setVisible(ui->actionChipEditorView->isChecked());
    if (ui->frameChipEditor->isVisible() && chip_opened_)
    {
        if (current_block_id_ == "") ui->chipEditorView->display_chip_level_info();
        else ui->chipEditorView->display_system_level_info(current_reg_id_, current_sig_id_);
    }
}

void RegisterManager::on_chipNavigator_chip_clicked()
{
    ui->docEditorView->set_doc_level(LEVEL::CHIP);
    ui->docEditorView->set_chip_id(chip_id_);
    ui->docEditorView->set_block_id("");
    if (ui->actionDocEditor->isChecked()) ui->docEditorView->display_documents();
    if (ui->actionDocPreview->isChecked()) ui->docEditorView->display_overall_documents();

    ui->chipEditorView->set_block_id("");
    if (ui->frameChipEditor->isVisible()) ui->chipEditorView->display_chip_level_info();
    current_block_id_ = "";
    current_reg_id_ = "";
    current_sig_id_ = "";
}

void RegisterManager::on_chipNavigator_block_clicked(QString block_id)
{
    ui->docEditorView->set_doc_level(LEVEL::BLOCK);
    ui->docEditorView->set_block_id(block_id);
    if (ui->frameDoc->isVisible() && ui->actionDocEditor->isChecked()) ui->docEditorView->display_documents();

    ui->chipEditorView->set_block_id(block_id);
    if (ui->frameChipEditor->isVisible()) ui->chipEditorView->display_system_level_info();
    current_block_id_ = block_id;
    current_reg_id_ = "";
    current_sig_id_ = "";
}

void RegisterManager::on_chipNavigator_register_clicked(QString block_id, QString reg_id)
{
    ui->docEditorView->set_doc_level(LEVEL::REGISTER);
    ui->docEditorView->set_block_id(block_id);
    ui->docEditorView->set_register_id(reg_id);
    if (ui->frameDoc->isVisible() && ui->actionDocEditor->isChecked()) ui->docEditorView->display_documents();

    ui->chipEditorView->set_block_id(block_id);
    if (ui->frameChipEditor->isVisible()) ui->chipEditorView->display_system_level_info(reg_id);
    current_block_id_ = block_id;
    current_reg_id_ = reg_id;
    current_sig_id_ = "";
}

void RegisterManager::on_chipNavigator_signal_clicked(QString block_id, QString sig_id)
{
    ui->docEditorView->set_doc_level(LEVEL::SIGNAL);
    ui->docEditorView->set_block_id(block_id);
    ui->docEditorView->set_signal_id(sig_id);
    if (ui->frameDoc->isVisible() && ui->actionDocEditor->isChecked()) ui->docEditorView->display_documents();

    ui->chipEditorView->set_block_id(block_id);
    if (ui->frameChipEditor->isVisible()) ui->chipEditorView->display_system_level_info("", sig_id);
    current_block_id_ = block_id;
    current_reg_id_ = "";
    current_sig_id_ = sig_id;
}

void RegisterManager::on_chipEditorView_chip_basics_edited(QString chip_name, QString chip_owner, QString chip_owner_id, int register_width, int address_width, bool msb_first)
{
    chip_name_ = chip_name;
    msb_first_ = msb_first;
    chip_owner_ = chip_owner;
    chip_owner_id_ = chip_owner_id;
    register_width_ = register_width;
    address_width_ = address_width;
    open_chip();
}

void RegisterManager::on_chipEditorView_block_added(QString block_id, QString block_name, QString block_abbr, QString responsible)
{
    ui->chipNavigator->add_block(block_id, block_name, block_abbr, responsible);
    set_completer();
    if (ui->actionDocPreview->isChecked()) ui->docEditorView->display_overall_documents();
}

void RegisterManager::on_chipEditorView_block_removed(int row)
{
    ui->chipNavigator->remove_block(row);
    set_completer();
    if (ui->actionDocPreview->isChecked()) ui->docEditorView->display_overall_documents();
}

void RegisterManager::on_chipEditorView_block_modified(int row, QString block_name, QString block_abbr, QString responsible)
{
    ui->chipNavigator->modify_block(row, block_name, block_abbr, responsible);
    set_completer();
    if (ui->actionDocPreview->isChecked()) ui->docEditorView->display_overall_documents();
}

void RegisterManager::on_chipEditorView_block_order_exchanged(int from, int to)
{
    ui->chipNavigator->change_block_order(from, to);
    if (ui->actionDocPreview->isChecked()) ui->docEditorView->display_overall_documents();
}

void RegisterManager::on_chipEditorView_to_refresh_navigator_block()
{
    ui->chipNavigator->refresh_block();
    set_completer();
    if (ui->actionDocPreview->isChecked()) ui->docEditorView->display_overall_documents();
}

void RegisterManager::LogMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
    case QtInfoMsg:
        txt = QString("[%1] Info: %2").arg(QDateTime::currentDateTime().toString(), msg);
        break;
    case QtDebugMsg:
        txt = QString("[%1] Debug: %2").arg(QDateTime::currentDateTime().toString(), msg);
        break;
    case QtWarningMsg:
        txt = QString("[%1] Warning: %2").arg(QDateTime::currentDateTime().toString(), msg);
        break;
    case QtCriticalMsg:
        txt = QString("[%1] Critical: %2").arg(QDateTime::currentDateTime().toString(), msg);
        break;
    case QtFatalMsg:
        txt = QString("[%1] Fatal: %2").arg(QDateTime::currentDateTime().toString(), msg);
        break;
    }

    QFile outFile(LOG_PATH);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream ts(&outFile);
        ts << txt << endl;
    }
}
