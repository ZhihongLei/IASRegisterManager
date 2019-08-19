#ifndef EDIT_NAMING_TEMPLATE_DIALOG_H
#define EDIT_NAMING_TEMPLATE_DIALOG_H

#include <QDialog>
#include <QSet>

namespace Ui {
class EditNamingTemplateDialog;
}

class EditNamingTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditNamingTemplateDialog(QWidget *parent = nullptr);
    ~EditNamingTemplateDialog();
    QString get_naming_template() const;

private slots:
    void on_pushButtonAdd_clicked();
    void on_pushButtonRemove_clicked();
    void on_comboBoxType_currentIndexChanged(int index);
    void on_listWidget_currentRowChanged(int currentRow);
    void on_lineEditConstant_textChanged(const QString &text);

private:
    void accept();
    bool sanity_check();
    bool check_name();
    bool check_delimiter();

    Ui::EditNamingTemplateDialog *ui;
    QSet<QString> reserved_types;
};

#endif // EDIT_NAMING_TEMPLATE_DIALOG_H
