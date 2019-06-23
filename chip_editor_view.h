#ifndef CHIP_EDITOR_VIEW_H
#define CHIP_EDITOR_VIEW_H

#include <QWidget>
#include <QMenu>
#include <QTableWidget>
#include "authenticator.h"

namespace Ui {
class ChipEditorView;
}

class ChipEditorView : public QWidget
{
    Q_OBJECT

public:
    explicit ChipEditorView(QWidget *parent = nullptr);
    ~ChipEditorView();

    void open_chip();
    void close_chip();
    void login(const QString& username, const QString& user_id);
    void open_chip(const QString& chip, const QString& chip_id, const QString& chip_owner, const QString& chip_owner_id,
                   int register_width, int address_width, bool msb_first);

    void display_chip_level_info();
    void display_system_level_info(const QString& reg_id="", const QString& sig_id="");

    void set_install_event_filter_register();

    void set_block_id(const QString& block_id);
    void set_authenticator(Authenticator* authenticator);

private slots:
    void on_tableSignal_customContextMenuRequested(QPoint pos);
    void on_tableRegister_customContextMenuRequested(QPoint pos);
    void on_tableSigPart_customContextMenuRequested(QPoint pos);
    void on_tableRegPart_customContextMenuRequested(QPoint pos);
    void on_tableSystem_customContextMenuRequested(QPoint pos);
    void on_tableDesigner_customContextMenuRequested(QPoint pos);
    void on_tableRegPage_customContextMenuRequested(QPoint pos);

    void on_actionAdd_triggered();
    void on_actionRemove_triggered();
    void on_actionEdit_triggered();
    void on_actionRefresh_triggered();

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
    void on_tableDesigner_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_tableRegPage_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void display_chip_basics();
    void display_system_blocks();
    void display_designers();
    void display_register_pages();
    void display_signals();
    void display_registers();
    void display_signal_partitions();
    void display_register_partitions();

signals:
    void block_added(QString block_id, QString block_name, QString block_abbr, QString responsible);
    void block_removed(int row);
    void block_modified(int row, QString block_name, QString block_abbr, QString responsible);
    void block_order_exchanged(int from, int to);
    void to_refresh_block();

private:
    bool eventFilter(QObject *obj, QEvent *eve);
    void table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row);
    Ui::ChipEditorView *ui;
    bool msb_first_;

    QAction *actionRemove_, *actionEdit_, *actionAdd_, *actionRefresh_;
    QMenu* context_menu_;
    Authenticator* authenticator_;
    QString user_id_, username_, chip_, chip_id_, block_, block_id_, chip_owner_, chip_owner_id_, previous_block_id_;
    int register_width_, address_width_;
};

#endif // CHIP_EDITOR_VIEW_H
