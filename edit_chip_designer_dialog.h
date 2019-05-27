#ifndef ADD_PROJECT_ROLE_DIALOG_H
#define ADD_PROJECT_ROLE_DIALOG_H

#include <QDialog>
#include "data_utils.h"

namespace Ui {
class EditChipDesignerDialog;
}

class EditChipDesignerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditChipDesignerDialog(const QString& chip_id, QWidget *parent = nullptr);
    explicit EditChipDesignerDialog(const QString& chip_id, const QString& chip_designer, const QString& project_role, bool enabled=true, QWidget *parent = nullptr);
    ~EditChipDesignerDialog();
    QString get_username() const;
    QString get_user_id() const;
    QString get_project_role() const;
    QString get_project_role_id() const;
    QString get_chip_designer_id() const;
    bool add_designer();
    bool edit_designer();

private:
    void setup_ui();
    void accept();
    bool sanity_check();
    Ui::EditChipDesignerDialog *ui;
    QVector<QString> project_role_ids_;
    const QString chip_id_;
    QString chip_designer_id_;
    const DIALOG_MODE mode_;
    const bool enabled_;

};

#endif // ADD_PROJECT_ROLE_DIALOG_H
