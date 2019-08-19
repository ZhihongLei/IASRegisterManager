#include "naming_template_dialog.h"
#include "ui_naming_template_dialog.h"
#include "edit_naming_template_dialog.h"
#include "global_variables.h"
#include <QSettings>

NamingTemplateDialog::NamingTemplateDialog(const QString& chip_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NamingTemplateDialog),
    chip_id_(chip_id)
{
    ui->setupUi(this);
    setWindowTitle("Naming Template");
    QSettings chip_settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "chip_settings");;
    chip_settings.beginGroup(chip_id_);

    if (chip_settings.value("register_naming_template").toString() != "")
        ui->lineEditRegNaming->setText(chip_settings.value("register_naming_template").toString());
    else ui->lineEditRegNaming->setText(DEFAULT_REGISTER_NAMING_TEMPLATE);

    if (chip_settings.value("signal_naming_template").toString() != "")
        ui->lineEditSigNaming->setText(chip_settings.value("signal_naming_template").toString());
    else ui->lineEditSigNaming->setText(DEFAULT_SIGNAL_NAMING_TEMPLATE);
}

NamingTemplateDialog::~NamingTemplateDialog()
{
    delete ui;
}

QString NamingTemplateDialog::get_signal_naming() const
{
    return ui->lineEditSigNaming->text();
}

QString NamingTemplateDialog::get_register_naming() const
{
    return ui->lineEditRegNaming->text();
}

void NamingTemplateDialog::on_pushButtonRegNaming_clicked()
{
    EditNamingTemplateDialog naming(this);
    if (naming.exec() == QDialog::Accepted) ui->lineEditRegNaming->setText(naming.get_naming_template());
}

void NamingTemplateDialog::on_pushButtonSigNaming_clicked()
{
    EditNamingTemplateDialog naming(this);
    if (naming.exec() == QDialog::Accepted)
        ui->lineEditSigNaming->setText(naming.get_naming_template());
}

