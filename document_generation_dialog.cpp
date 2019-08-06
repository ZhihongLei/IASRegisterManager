#include "document_generation_dialog.h"
#include "ui_document_generation_dialog.h"
#include "document_generator.h"
#include "naming_template_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include "data_utils.h"
#include "authenticator.h"
#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

DocumentGenerationDialog::DocumentGenerationDialog(const QString& chip_id,
                                  const QString& chip_name,
                                  int register_width,
                                  int address_width,
                                  bool msb_first,
                                  Authenticator* authenticator,
                                  const QString& user_id,
                                  QWidget *parent):
    QDialog(parent),
    ui(new Ui::DocumentGenerationDialog),
    chip_id_(chip_id),
    chip_name_(chip_name),
     user_id_(user_id),
    register_width_(register_width),
    address_width_(address_width),
    msb_first_(msb_first),
    authenticator_(authenticator)
{
    ui->setupUi(this);
    QSettings chip_setttings("chip_setttings.ini", QSettings::IniFormat);
    chip_setttings.beginGroup(chip_id_);

    if (chip_setttings.value("register_naming_template").toString() != "")
        ui->lineEditRegNaming->setText(chip_setttings.value("register_naming_template").toString());
    else ui->lineEditRegNaming->setText(DEFAULT_REGISTER_NAMING_TEMPLATE);

    if (chip_setttings.value("signal_naming_template").toString() != "")
        ui->lineEditSigNaming->setText(chip_setttings.value("signal_naming_template").toString());
    else ui->lineEditSigNaming->setText(DEFAULT_SIGNAL_NAMING_TEMPLATE);

    if (chip_setttings.value("image_caption_position").toString() == "top")
        ui->radioButtonImgCapTop->setChecked(true);
    else ui->radioButtonImgCapBottom->setChecked(true);

    if (chip_setttings.value("table_caption_position").toString() == "bottom")
        ui->radioButtonTabCapBottom->setChecked(true);
    else ui->radioButtonTabCapTop->setChecked(true);

    if (chip_setttings.value("show_paged_register").toString() == "page_name")
        ui->radioButtonPageName->setChecked(true);
    else ui->radioButtonPageControlSignal->setChecked(true);

    ui->comboBoxDocType->addItem("LaTeX");
    ui->comboBoxDocType->addItem("HTML");

    QVector<QVector<QString> > items;
    bool success = true;
    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator->can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id}, {"block_system_block.responsible", user_id}};
    success = success && DataBaseHandler::show_items_inner_join({"block_system_block.block_id",
                                     "block_system_block.block_name",
                                     "block_system_block.abbreviation",
                                    "block_system_block.start_address", "block_system_block.prev", "block_system_block.next"},
                                    {{{"block_system_block", "responsible"}, {"global_user", "user_id"}}},
                                    items, key_value_pairs,
                                    "order by block_system_block.block_id");
    if (authenticator->can_read_all_blocks()) items = sort_doubly_linked_list(items);

    ui->listWidgetToExport->addItem(chip_name);
    check_states_.push_back(Qt::CheckState::Unchecked);
    for (const auto& item : items)
    {
        block_ids_.push_back(item[0]);
        block_names_.push_back(item[1]);
        block_abbrs_.push_back(item[2]);
        block_start_addrs_.push_back(item[3]);
        ui->listWidgetToExport->addItem(item[1]);
        check_states_.push_back(Qt::CheckState::Unchecked);
    }
    ui->checkBoxSelectAll->setChecked(true);

    setWindowTitle("Document Generation");
    if (!success) QMessageBox::warning(this, windowTitle(), "Unable to initialize due to database connection issue.\nPlease try again!");
}

DocumentGenerationDialog::~DocumentGenerationDialog()
{
    delete ui;
}

QString DocumentGenerationDialog::get_register_naming() const
{
    return ui->lineEditRegNaming->text();
}

QString DocumentGenerationDialog::get_signal_naming() const
{
    return ui->lineEditSigNaming->text();
}

QString DocumentGenerationDialog::get_image_caption_position() const
{
    return ui->radioButtonImgCapTop->isChecked() ? "top" : "bottom";
}

QString DocumentGenerationDialog::get_table_caption_position() const
{
    return ui->radioButtonTabCapTop->isChecked() ? "top" : "bottom";
}

QString DocumentGenerationDialog::get_show_paged_register() const
{
    return ui->radioButtonPageName->isChecked() ? "page_name" : "control_signal";
}

void DocumentGenerationDialog::on_comboBoxDocType_currentIndexChanged(int index)
{
    ui->lineEditPath->clear();
}

void DocumentGenerationDialog::on_pushButtonRegNaming_clicked()
{
    NamingTemplateDialog naming;
    if (naming.exec() == QDialog::Accepted) ui->lineEditRegNaming->setText(naming.get_naming_template());
}

void DocumentGenerationDialog::on_pushButtonSigNaming_clicked()
{
    NamingTemplateDialog naming;
    if (naming.exec() == QDialog::Accepted)
        ui->lineEditSigNaming->setText(naming.get_naming_template());
}

void DocumentGenerationDialog::on_listWidgetToExport_itemClicked(QListWidgetItem *item)
{
    if (!item) return;
    if (item->checkState() == Qt::CheckState::Checked) item->setCheckState(Qt::CheckState::Unchecked);
    else item->setCheckState(Qt::CheckState::Checked);
}

void DocumentGenerationDialog::on_checkBoxSelectAll_toggled(bool checked)
{
    if (checked)
    {
        for (int i = 0; i < ui->listWidgetToExport->count(); i++)
        {
            check_states_[i] = ui->listWidgetToExport->item(i)->checkState();
            ui->listWidgetToExport->item(i)->setCheckState(Qt::CheckState::Checked);
        }
    }
    else {
        for (int i = 0; i < ui->listWidgetToExport->count(); i++)
            ui->listWidgetToExport->item(i)->setCheckState(check_states_[i]);
    }
}

void DocumentGenerationDialog::on_pushButtonSelectPath_clicked()
{
    QString path = ui->comboBoxDocType->currentIndex() == 0 ? QFileDialog::getSaveFileName(this, "Export LaTeX Document", "", "text files (*.tex)"):
                                                              QFileDialog::getSaveFileName(this, "Export HTML Document", "", "HTML files (*.html)");
    ui->lineEditPath->setText(path);
}

void DocumentGenerationDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool DocumentGenerationDialog::sanity_check()
{
    if (ui->lineEditPath->text() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify a valid path!");
        return false;
    }
    bool selected = false;
    for (int i = 0; i < ui->listWidgetToExport->count(); i++)
        if (ui->listWidgetToExport->item(i)->checkState() == Qt::CheckState::Checked)
        {
            selected = true;
            break;
        }
    if (!selected)
    {
        QMessageBox::warning(this, windowTitle(), "Nothing to export!");
        return false;
    }
    return true;
}

bool DocumentGenerationDialog::generate_document()
{
    DocumentGenerator generator(chip_id_, chip_name_, address_width_, register_width_,
                                msb_first_, user_id_, authenticator_,
                                ui->radioButtonImgCapTop->isChecked() ? DocumentGenerator::TOP : DocumentGenerator::BOTTOM,
                                ui->radioButtonTabCapTop->isChecked() ? DocumentGenerator::TOP : DocumentGenerator::BOTTOM,
                                get_show_paged_register());


    QString doc, table_of_content;
    QString chip_doc;
    if (ui->listWidgetToExport->item(0)->checkState() == Qt::CheckState::Checked)
        chip_doc = ui->comboBoxDocType->currentIndex() == 0 ? generator.generate_chip_level_tex_document() :
                                                                generator.generate_chip_level_html_document();

    auto reg_id2page = generator.get_register_id2page();
    for (int i = 1; i < ui->listWidgetToExport->count(); i++)
    {
        if (ui->listWidgetToExport->item(i)->checkState() != Qt::CheckState::Checked) continue;
        if (ui->comboBoxDocType->currentIndex() == 0)
            doc += generator.generate_block_level_tex_document(block_ids_[i-1], block_names_[i-1], block_abbrs_[i-1],
                                                                        block_start_addrs_[i-1], reg_id2page);
        else {
            QString block_content = generator.generate_block_level_html_document(block_ids_[i-1], block_names_[i-1], block_abbrs_[i-1],
                                                                                    block_start_addrs_[i-1], reg_id2page);
            QString reg_bullets = block_content.split(DOC_DELIMITER)[0];
            block_content = block_content.right(block_content.size() - reg_bullets.size() - DOC_DELIMITER.size());
            doc += block_content;
            table_of_content += QString("<li><a href=#${BLOCK_NAME}>${BLOCK_NAME}</a></li>\n").replace("${BLOCK_NAME}", block_names_[i-1]) + reg_bullets;
        }
    }
    if (ui->comboBoxDocType->currentIndex() == 1) // html
        doc = chip_doc + "<ol>\n" + table_of_content + "</ol>\n" + doc;
    else doc = chip_doc + doc;
    if (!generator.success())
    {
        QMessageBox::warning(this, windowTitle(), "Unable to generate document.\nError message: " + DataBaseHandler::get_error_message() + "!");
        return false;
    }

    QString path = ui->lineEditPath->text();
    QFile file(path);
    if( !file.open(QIODevice::WriteOnly) )
    {
      QMessageBox::warning(this, windowTitle(), "Unable to export document due to IO error.\nPlease try again!");
      return false;
    }
    QTextStream outputStream(&file);
    outputStream <<  doc;
    file.close();
    return true;
}
