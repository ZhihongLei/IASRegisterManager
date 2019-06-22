#ifndef DOCUMENT_EDITOR_VIEW_H
#define DOCUMENT_EDITOR_VIEW_H

#include <QWidget>
#include <QMenu>
#include <QTableWidget>
#include "authenticator.h"
#include "register_manager.h"
#include "data_utils.h"

namespace Ui {
class DocumentEditorView;
}

class DocumentEditorView : public QWidget
{
    Q_OBJECT

public:
    explicit DocumentEditorView(QWidget *parent = nullptr);
    ~DocumentEditorView();
    void display_documents();
    void display_overall_documents();
    void set_authenticator(Authenticator* authenticator);
    void set_doc_level(const LEVEL& level);
    void set_register_width(int width);
    void set_address_width(int width);
    void set_msb_first(bool msb_first);
    void set_user_id(const QString& user_id);
    void set_chip_id(const QString& chip_id);
    void set_block_id(const QString& block_id);
    void set_register_id(const QString& register_id);
    void set_signal_id(const QString& signal_id);
    void set_install_event_filter();

    void close_chip();

private slots:
    void on_tableDoc_customContextMenuRequested(QPoint pos);
    void on_document_edited();
    void on_document_added();

    void on_pushButtonAddDoc_clicked();
    void on_pushButtonRemoveDoc_clicked();
    void on_tableDoc_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_tableDoc_cellDoubleClicked(int row, int column);
    void on_stackedWidgetDoc_currentChanged(int index);

    void on_actionAdd_triggered();
    void on_actionRemove_triggered();
    void on_actionEdit_triggered();
    void on_actionRefresh_triggered();

private:
    bool eventFilter(QObject *obj, QEvent *eve);
    void table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row);

    Ui::DocumentEditorView *ui;
    QAction *actionRemove_, *actionEdit_, *actionAdd_, *actionRefresh_;
    QMenu* context_menu_;
    Authenticator *authenticator_;
    LEVEL level_;
    QString chip_id_, block_id_, register_id_, signal_id_;
    QString user_id_;
    bool msb_first_;
    int register_width_, address_width_;
};

#endif // DOCUMENT_EDITOR_VIEW_H
