#ifndef EDIT_CHIP_DIALOG_H
#define EDIT_CHIP_DIALOG_H

#include <QDialog>

namespace Ui {
class EditChipDialog;
}

class EditChipDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditChipDialog(const QString& creater_id, QWidget *parent = nullptr);
    ~EditChipDialog();
    QString get_chip_name() const;
    QString get_chip_id() const;
    QString get_chip_designer_id() const;
    QString get_project_role() const;
    QString get_project_role_id() const;
    int get_register_width() const;
    int get_address_width() const;

    bool add_chip();

private:
    void accept();
    bool sanity_check();
    bool check_name();
    bool check_register_width();
    bool check_address_width();

    Ui::EditChipDialog *ui;
    const QString creater_id_;
    QString chip_id_, chip_designer_id_, project_role_, project_role_id_;
};

#endif // EDIT_CHIP_DIALOG_H
