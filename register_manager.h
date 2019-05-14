#ifndef REGISTER_MANAGER_H
#define REGISTER_MANAGER_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QHash>
#include <QTableWidget>
#include "authenticator.h"


namespace Ui {
class RegisterManager;
}

class RegisterManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit RegisterManager(QWidget *parent = nullptr);
    ~RegisterManager();
    const QString& get_username() const;
    const QString& get_user_id() const;
    const QString& get_db_role() const;
    const QString& get_db_role_id() const;

private slots:
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_user_management();
    void on_change_password();
    void on_new_chip();
    void on_open_chip();

    void on_pushButtonAddSys_clicked();

    void on_pushButtonAddDesigner_clicked();

    void on_pushButtonRemoveSys_clicked();

    void on_pushButtonRemoveDesigner_clicked();

    void on_pushButtonAddSig_clicked();

    void on_pushButtonRemoveSig_clicked();

    void on_pushButtonAddReg_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_pushButtonRemoveReg_clicked();

    void on_pushButtonAddSigPart_clicked();

    void on_pushButtonRemoveSigPart_clicked();

    void on_tableSignal_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_tableReg_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_tableSignal_cellDoubleClicked(int row, int column);

    void on_tableReg_cellDoubleClicked(int row, int column);

    void on_tableSystem_cellDoubleClicked(int row, int column);

public slots:
    void on_loggedin(QString);

private:
    Ui::RegisterManager *ui;
    QString username_, user_id_, db_role_, db_role_id_, chip_, chip_id_, block_, block_id_, chip_owner_id_, chip_owner_;
    int address_width_, register_width_;
    QVector<QString> blocks_;
    Authenticator authenticator_;
    void init_db();
    void clear_db();
    void open_chip();
    void display_system_blocks();
    void display_designers();
    void display_register_pages();
    void display_signals();
    void display_registers();
    void display_signal_partitions(const QString& reg_sig_id);
    void display_register_partitions(const QString& reg_id);

    bool eventFilter(QObject *obj, QEvent *eve);
    void table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row);
    bool msb_first_;
};

#endif // REGISTER_MANAGER_H
