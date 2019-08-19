#ifndef DOCUMENT_EDITOR_VIEW_H
#define DOCUMENT_EDITOR_VIEW_H

#include <QWidget>
#include "data_utils.h"

namespace Ui {
class DocumentEditorView;
}

class QCompleter;
class QMenu;
class QTableWidget;
class Authenticator;

class DocumentEditorView : public QWidget
{
    Q_OBJECT

public:
    enum DOCUMENT_VIEW {PREVIEW, EDITOR_VIEW};
    explicit DocumentEditorView(QWidget *parent = nullptr);
    ~DocumentEditorView();

    void login(const QString& username, const QString& user_id);
    void open_chip(const QString& chip, const QString& chip_id,
                   int register_width, int address_width, bool msb_first);
    void close_chip();
    void set_authenticator(Authenticator* authenticator);
    void set_doc_level(const LEVEL& level);
    void set_view(const DOCUMENT_VIEW& view);

    void set_chip_id(const QString& chip_id);
    void set_block_id(const QString& block_id);
    void set_register_id(const QString& register_id);
    void set_signal_id(const QString& signal_id);
    void set_completer(QCompleter* c);

    void display_documents();
    void display_overall_documents();

private slots:
    void on_tableDoc_customContextMenuRequested(QPoint pos);

    void to_add();
    void to_remove();
    void to_edit();
    void to_copy();
    void to_paste();
    void to_refresh();

    void on_stackedWidgetDoc_currentChanged(int index);
    void on_pushButtonAddDoc_clicked();
    void on_pushButtonRemoveDoc_clicked();
    void on_tableDoc_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_tableDoc_cellDoubleClicked(int row, int column);

    void document_added();
    void document_edited();

private:
    void set_install_event_filter(bool enabled=true);
    bool eventFilter(QObject *obj, QEvent *eve);
    void table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row);

    Ui::DocumentEditorView *ui;
    QAction *actionRemove_, *actionEdit_, *actionAdd_, *actionRefresh_, *actionCopy_, *actionPaste_;
    QMenu* context_menu_;
    Authenticator *authenticator_;
    LEVEL level_;
    QString username_, user_id_, chip_name_, chip_id_, block_id_, register_id_, signal_id_;
    bool msb_first_;
    int register_width_, address_width_;
    QString copied_;
};

#endif // DOCUMENT_EDITOR_VIEW_H
