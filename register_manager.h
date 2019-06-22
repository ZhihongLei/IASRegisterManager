#ifndef REGISTER_MANAGER_H
#define REGISTER_MANAGER_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QHash>
#include <QTableWidget>
#include "authenticator.h"
#include <QSplitter>
#include "login_dialog.h"


namespace Ui {
class RegisterManager;
}

class RegisterManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit RegisterManager(QWidget *parent = nullptr);
    ~RegisterManager();

private slots:
    void on_treeWidgetBlock_itemClicked(QTreeWidgetItem *item, int column);
    void on_actionUserManagement_triggered();
    void on_actionChangePassword_triggered();
    void on_actionLogOut_triggered();

    void on_actionNewChip_triggered();
    void on_actionOpenChip_triggered();
    void on_actionCloseChip_triggered();
    void on_actionChipManagement_triggered();

    void on_actionDocEditorView_triggered();
    void on_actionChipEditorView_triggered();

    void on_actionAdd_triggered();
    void on_actionRemove_triggered();
    void on_actionEdit_triggered();
    void on_actionRefresh_triggered();

    //void on_treeWidgetDoc_customContextMenuRequested(QPoint pos);
    void on_tableSignal_customContextMenuRequested(QPoint pos);
    void on_tableRegister_customContextMenuRequested(QPoint pos);
    void on_tableSigPart_customContextMenuRequested(QPoint pos);
    void on_tableRegPart_customContextMenuRequested(QPoint pos);
    void on_tableSystem_customContextMenuRequested(QPoint pos);
    void on_tableDesigner_customContextMenuRequested(QPoint pos);
    void on_tableRegPage_customContextMenuRequested(QPoint pos);

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

    void on_tableRegister_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_tableSignal_cellDoubleClicked(int row, int column);

    void on_tableRegister_cellDoubleClicked(int row, int column);

    void on_tableSystem_cellDoubleClicked(int row, int column);

    void on_tableSystem_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_tableSigPart_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_pushButtonAddRegPage_clicked();

    void on_pushButtonRemoveRegPage_clicked();

    void on_tableRegPage_cellDoubleClicked(int row, int column);

    void on_tableDesigner_cellDoubleClicked(int row, int column);

    void on_lineEditSearch_editingFinished();

    void on_tableDesigner_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_lineEditSearch_textChanged(const QString &arg1);

    void on_tableRegPage_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

public slots:
    void on_loggedin(QString);

signals:
    void contextMenuRequested();

private:
    Ui::RegisterManager *ui;
    QString username_, user_id_, db_role_, db_role_id_, chip_, chip_id_, block_, block_id_, chip_owner_id_, chip_owner_;
    int address_width_, register_width_;
    QVector<QString> blocks_;
    QHash<QString, QString> block_id2abbr_;
    Authenticator authenticator_;
    void init_db();
    void clear_db();
    void open_chip();
    void display_chip_basics();
    void display_system_blocks();
    void display_designers();
    void display_register_pages();
    void display_signals();
    void display_registers();
    void display_signal_partitions();
    void display_register_partitions();
    void refresh_block(QTreeWidgetItem* block_item);

    bool search(QTreeWidgetItem* item, const QString& s, bool visible);

    bool eventFilter(QObject *obj, QEvent *eve);
    void table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row);
    bool msb_first_;

    QAction *actionRemove_, *actionEdit_, *actionAdd_, *actionRefresh_;
    QMenu* context_menu_;
    LoginDialog login_dialog_;
};

#endif // REGISTER_MANAGER_H
