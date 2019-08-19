#include "edit_naming_template_dialog.h"
#include "ui_edit_naming_template_dialog.h"
#include <QMessageBox>

EditNamingTemplateDialog::EditNamingTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditNamingTemplateDialog)
{
    ui->setupUi(this);
    for (QString type : {"${CHIP_NAME}", "${BLOCK_NAME}", "${BLOCK_ABBR}", "${GIVEN_NAME}"})
    {
        ui->comboBoxType->addItem(type);
        reserved_types.insert(type);
    }
    ui->comboBoxType->addItem("CONSTANT");
    ui->pushButtonAdd->setEnabled(true);
    ui->pushButtonRemove->setEnabled(false);
    ui->lineEditConstant->setVisible(false);
    ui->lineEditDelimiter->setText("_");
    setWindowTitle("Edit Naming Template");
}

EditNamingTemplateDialog::~EditNamingTemplateDialog()
{
    delete ui;
}

QString EditNamingTemplateDialog::get_naming_template() const
{
    QString naming_template;
    QString delimiter = ui->lineEditDelimiter->text();
    for (int row = 0; row < ui->listWidget->count(); row++)
    {
        QString name = ui->listWidget->item(row)->text();
        if (row > 0) naming_template += delimiter;
        naming_template += name;
    }
    return naming_template;
}

void EditNamingTemplateDialog::on_pushButtonAdd_clicked()
{
    int index = ui->comboBoxType->currentIndex();
    QString type;
    if (index != ui->comboBoxType->count() - 1)
    {
        type = ui->comboBoxType->itemText(index);
        ui->comboBoxType->removeItem(index);
    }
    else
    {
        type = ui->lineEditConstant->text();
        if (reserved_types.contains(type))
        {
            QMessageBox::warning(this, windowTitle(), type + " is a reserved type!");
            return;
        }
    }
    ui->listWidget->addItem(type);
}

void EditNamingTemplateDialog::on_pushButtonRemove_clicked()
{
    int row = ui->listWidget->currentRow();
    QString type = ui->listWidget->item(row)->text();
    if (reserved_types.contains(type)) ui->comboBoxType->insertItem(0, type);
    ui->listWidget->takeItem(row);
}

void EditNamingTemplateDialog::on_comboBoxType_currentIndexChanged(int index)
{
    ui->lineEditConstant->setVisible(index >= 0 && index == ui->comboBoxType->count() - 1);
    ui->pushButtonAdd->setEnabled(ui->comboBoxType->currentIndex() != ui->comboBoxType->count() - 1 ||
            ui->lineEditConstant->text() != "");
}

void EditNamingTemplateDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->pushButtonRemove->setEnabled(currentRow >= 0);
}

void EditNamingTemplateDialog::on_lineEditConstant_textChanged(const QString &text)
{
    ui->pushButtonAdd->setEnabled(ui->comboBoxType->currentIndex() != ui->comboBoxType->count() - 1 || text != "");
}

void EditNamingTemplateDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool EditNamingTemplateDialog::sanity_check()
{
    return check_name() && check_delimiter();
}

bool EditNamingTemplateDialog::check_name()
{
    bool has_given_name = false;
    for (int row = 0; row < ui->listWidget->count(); row++)
    {
        if (ui->listWidget->item(row)->text() == "${GIVEN_NAME}")
        {
            has_given_name = true;
            break;
        }
    }
    if (!has_given_name)
    {
        QMessageBox::warning(this, windowTitle(), "GIVEN NAME must be added in the list!");
        return false;
    }
    return true;
}

bool EditNamingTemplateDialog::check_delimiter()
{
    QString delimiter = ui->lineEditDelimiter->text();
    if (delimiter == "")
    {
        QMessageBox::warning(this, windowTitle(), "Delimiter must not be empty!");
        return false;
    }
    return true;
}
