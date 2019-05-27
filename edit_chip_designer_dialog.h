#ifndef ADD_PROJECT_ROLE_DIALOG_H
#define ADD_PROJECT_ROLE_DIALOG_H

#include <QDialog>
namespace Ui {
class AddChipDesignerDialog;
}

class AddChipDesignerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddChipDesignerDialog(const QString& chip_id, QWidget *parent = nullptr);
    ~AddChipDesignerDialog();
    QString get_username() const;
    QString get_user_id() const;
    QString get_project_role() const;
    QString get_project_role_id() const;
    QString get_chip_designer_id() const;
    bool add_designer();

private:
    void accept();
    bool sanity_check();
    Ui::AddChipDesignerDialog *ui;
    QVector<QString> project_role_ids_;
    const QString chip_id_;
    QString chip_designer_id_;

};

#endif // ADD_PROJECT_ROLE_DIALOG_H
