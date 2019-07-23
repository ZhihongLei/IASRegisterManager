#ifndef OPEN_CHIP_DIALOG_H
#define OPEN_CHIP_DIALOG_H

#include <QDialog>

namespace Ui {
class OpenChipDialog;
}

class OpenChipDialog : public QDialog
{
    Q_OBJECT

public:
    enum DIALOG_MODE {OPEN_CHIP, MANAGE_CHIP};
    explicit OpenChipDialog(const QString& username, const QString& user_id, bool can_add_chip, QWidget *parent = nullptr);
    explicit OpenChipDialog(const QString& username, const QString& user_id, QString active_chip_id, QWidget *parent = nullptr);
    ~OpenChipDialog();
    QString get_chip_id() const;
    QString get_chip_name() const;
    QString get_owner() const;
    QString get_owner_id() const;
    QString get_project_role_id() const;
    int get_register_width() const;
    int get_address_width() const;
    bool msb_first() const;
    bool frozen() const;

private slots:
    void on_pushButtonAddChip_clicked();
    void on_pushButtonRemoveChip_clicked();
    void on_tableWidgetChip_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_tableWidgetChip_cellDoubleClicked(int row, int column);

private:
    bool setup_ui();
    void accept();
    bool sanity_check();
    bool check_project_role();

    Ui::OpenChipDialog *ui;
    const QString username_, user_id_, active_chip_id_;
    QString project_role_id_;
    QVector<QString> chip_ids_;
    int address_width_, register_width_;
    const DIALOG_MODE mode_;
};

#endif // OPEN_CHIP_DIALOG_H
