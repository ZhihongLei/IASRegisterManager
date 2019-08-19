#ifndef NAMING_TEMPLATE_DIALOG_H
#define NAMING_TEMPLATE_DIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class NamingTemplateDialog;
}

class NamingTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NamingTemplateDialog(const QString& chip_id, QWidget *parent = nullptr);
    QString get_signal_naming() const;
    QString get_register_naming() const;
    ~NamingTemplateDialog();

private slots:
    void on_pushButtonRegNaming_clicked();

    void on_pushButtonSigNaming_clicked();

private:
    Ui::NamingTemplateDialog *ui;
    const QString chip_id_;
};

#endif // NAMING_TEMPLATE_DIALOG_H
