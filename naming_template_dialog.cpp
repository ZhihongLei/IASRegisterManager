#include "naming_template_dialog.h"
#include "ui_naming_template_dialog.h"
#include <QMessageBox>

NamingTemplateDialog::NamingTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NamingTemplateDialog)
{
    ui->setupUi(this);
    for (QString type : {"CHIP_NAME", "BLOCK_NAME", "BLOCK_ABBR", "GIVEN_NAME", "Other"})
    {
        ui->comboBoxType->addItem(type);
        reserved_types.insert(type);
    }
    ui->pushButtonAdd->setEnabled(true);
    ui->pushButtonRemove->setEnabled(false);
    ui->lineEditOther->setVisible(false);
    ui->lineEditDelimiter->setText("_");
    setWindowTitle("Naming Template");
}

NamingTemplateDialog::~NamingTemplateDialog()
{
    delete ui;
}

QString NamingTemplateDialog::get_naming_template() const
{
    QString naming_template;
    QString delimiter = ui->lineEditDelimiter->text();
    for (int row = 0; row < ui->listWidget->count(); row++)
    {
        QString name = ui->listWidget->item(row)->text();
        if (reserved_types.contains(name)) name = "${" + name + "}";
        if (row > 0) naming_template += delimiter;
        naming_template += name;
    }
    return naming_template;
}

void NamingTemplateDialog::on_pushButtonAdd_clicked()
{
    int index = ui->comboBoxType->currentIndex();
    QString type;
    if (index != ui->comboBoxType->count() - 1)
    {
        type = ui->comboBoxType->itemText(index);
        ui->comboBoxType->removeItem(index);
    }
    else type = ui->lineEditOther->text();
    ui->listWidget->addItem(type);
}

void NamingTemplateDialog::on_pushButtonRemove_clicked()
{
    int row = ui->listWidget->currentRow();
    QString type = ui->listWidget->item(row)->text();
    if (reserved_types.contains(type)) ui->comboBoxType->insertItem(0, type);
    ui->listWidget->takeItem(row);
}

void NamingTemplateDialog::on_comboBoxType_currentIndexChanged(int index)
{
    ui->lineEditOther->setVisible(index >= 0 && index == ui->comboBoxType->count() - 1);
    ui->pushButtonAdd->setEnabled(ui->comboBoxType->currentIndex() != ui->comboBoxType->count() - 1 ||
            ui->lineEditOther->text() != "");
}

void NamingTemplateDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->pushButtonRemove->setEnabled(currentRow >= 0);
}

void NamingTemplateDialog::on_lineEditOther_textChanged(const QString &text)
{
    ui->pushButtonAdd->setEnabled(ui->comboBoxType->currentIndex() != ui->comboBoxType->count() - 1 || text != "");
}

void NamingTemplateDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool NamingTemplateDialog::sanity_check()
{
    return check_name() && check_delimiter();
}

bool NamingTemplateDialog::check_name()
{
    bool has_given_name = false;
    for (int row = 0; row < ui->listWidget->count(); row++)
    {
        if (ui->listWidget->item(row)->text() == "GIVEN_NAME")
        {
            has_given_name = true;
            break;
        }
    }
    if (!has_given_name)
    {
        QMessageBox::warning(this, "Naming Template", "GIVEN NAME must be added in the list!");
        return false;
    }
    return true;
}

bool NamingTemplateDialog::check_delimiter()
{
    QString delimiter = ui->lineEditDelimiter->text();
    if (delimiter == "")
    {
        QMessageBox::warning(this, "Naming Template", "Delimiter must not be empty!");
        return false;
    }
    return true;
}
