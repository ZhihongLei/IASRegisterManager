#ifndef ENCRYPTER_DIALOG_H
#define ENCRYPTER_DIALOG_H

#include <QDialog>

namespace Ui {
class EncrypterDialog;
}

class EncrypterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EncrypterDialog(QWidget *parent = nullptr);
    ~EncrypterDialog();

private slots:
    void on_pushButtonGenerate_clicked();
    void on_plainTextEdit_textChanged();
    void on_lineEditKey_textChanged();
    void on_pushButtonEncrypt_clicked();

private:
    Ui::EncrypterDialog *ui;
};

#endif // ENCRYPTER_DIALOG_H
