#ifndef NAMING_TEMPLATE_DIALOG_H
#define NAMING_TEMPLATE_DIALOG_H

#include <QDialog>
#include <QSet>

namespace Ui {
class NamingTemplateDialog;
}

class NamingTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NamingTemplateDialog(QWidget *parent = nullptr);
    ~NamingTemplateDialog();
    QString get_naming_template() const;

private slots:
    void on_pushButtonAdd_clicked();
    void on_pushButtonRemove_clicked();
    void on_comboBoxType_currentIndexChanged(int index);
    void on_listWidget_currentRowChanged(int currentRow);
    void on_lineEditOther_textChanged(const QString &text);

private:
    void accept();
    bool sanity_check();
    bool check_name();
    bool check_delimiter();

    Ui::NamingTemplateDialog *ui;
    QSet<QString> reserved_types;
};

#endif // NAMING_TEMPLATE_DIALOG_H
