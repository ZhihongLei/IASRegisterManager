#ifndef EDIT_SYSTEM_BLOCK_DIALOG_H
#define EDIT_SYSTEM_BLOCK_DIALOG_H

#include <QDialog>
#include "data_utils.h"

namespace Ui {
class EditSystemBlockDialog;
}

class EditSystemBlockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditSystemBlockDialog(const QString& chip_id, int address_width, bool can_add_designer, QWidget *parent = nullptr);
    explicit EditSystemBlockDialog(const QString& chip_id, const QString& block_id, int address_width, bool can_add_designer, bool enabled=true, QWidget *parent = nullptr);
    ~EditSystemBlockDialog();
    QString get_block_name() const;
    QString get_block_id() const;
    QString get_block_abbr() const;
    QString get_start_addr() const;
    QString get_responsible() const;
    QString get_responsible_id() const;
    bool designer_added() const;
    bool add_system_block();
    bool edit_system_block();


private slots:
    void on_pushButtonAddDesigner_clicked();

private:
    bool setup_ui();
    void accept();
    bool sanity_check();
    bool check_block_name();
    bool check_block_abbreviation();
    bool check_start_address();

    Ui::EditSystemBlockDialog *ui;
    QString chip_id_, block_id_;
    const int address_width_;
    QHash<QString, QString> responsible2user_id_;
    QString original_block_name_, original_abbr_;
    const DIALOG_MODE mode_;
    bool designer_added_ = false;
    const bool enabled_;
};

#endif // EDIT_SYSTEM_BLOCK_DIALOG_H
