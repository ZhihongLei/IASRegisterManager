#include "encrypter_dialog.h"
#include "ui_encrypter_dialog.h"
#include <QCryptographicHash>
#include "../qaesencryption.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QDebug>


EncrypterDialog::EncrypterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EncrypterDialog)
{
    ui->setupUi(this);
    ui->pushButtonEncrypt->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

EncrypterDialog::~EncrypterDialog()
{
    delete ui;
}

void EncrypterDialog::on_pushButtonGenerate_clicked()
{
    ui->lineEditKey->setText(QAESEncryption::generate_key());
}

void EncrypterDialog::on_pushButtonEncrypt_clicked()
{
    QString plainText = ui->plainTextEdit->toPlainText();
    QString key = ui->lineEditKey->text();
    if (plainText == "" || key == "")
    {
        QMessageBox::warning(this, "IAS Encrypter", "Plaintext and key must not be empty!");
        return;
    }

    QString encryptedText = QAESEncryption::encode(plainText, key);
    QString decodedText = QAESEncryption::decode(encryptedText, key);

    if (decodedText == plainText)
    {
        ui->entryptedTextEdit->setPlainText(encryptedText);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else QMessageBox::warning(this, "IAS Encrypter", "Failed to encrypt text!");
}


void EncrypterDialog::on_plainTextEdit_textChanged()
{
    ui->pushButtonEncrypt->setEnabled(ui->plainTextEdit->toPlainText() != "" && ui->lineEditKey->text() != "");
    ui->entryptedTextEdit->setPlainText("");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void EncrypterDialog::on_lineEditKey_textChanged()
{
    ui->pushButtonEncrypt->setEnabled(ui->plainTextEdit->toPlainText() != "" && ui->lineEditKey->text() != "");
    ui->entryptedTextEdit->setPlainText("");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

