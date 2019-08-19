#ifndef DOCUMENT_EDITOR_H
#define DOCUMENT_EDITOR_H

#include <QWidget>
#include <QStyledItemDelegate>
#include "data_utils.h"

class QCompleter;
class LineEdit;

namespace Ui {
class EditDocumentDialog;
}

class EditDocumentDialog : public QWidget
{
    Q_OBJECT

public:
    explicit EditDocumentDialog(QWidget *parent = nullptr);
    ~EditDocumentDialog();

    void set_mode(const DIALOG_MODE& mode);
    void set_doc_level(const LEVEL& level);
    void set_completer(QCompleter* c);
    void setEnabled(bool enabled);
    void set_chip_id(const QString& chip_id);
    void set_block_id(const QString& block_id);
    void set_register_id(const QString& reg_id);
    void set_signal_id(const QString& sig_id);
    void set_content(const QString& doc_id, const QString& doc_type, const QString& content);
    void clear_content();
    void clear();

    QString get_document_type() const;
    QString get_document_type_id() const;
    QString get_doc_id() const;
    QString get_content() const;

    bool add_document();
    bool edit_document();

signals:
    void to_add_document();
    void to_edit_document();

private slots:
    void on_comboBoxDocType_currentIndexChanged(int index);
    void on_pushButtonPreview_clicked();

    void on_textEditText_textChanged();
    void on_pushButtonSelectImage_clicked();
    void on_lineEditImageCaption_editingFinished();
    void on_lineEditImageWidth_editingFinished();
    void on_lineEditTableRow_editingFinished();
    void on_lineEditTableColumn_editingFinished();
    void on_tableWidget_cellDoubleClicked(int row, int column);
    void on_tableWidget_cellChanged(int row, int column);
    void on_lineEditTableCaption_editingFinished();

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    bool setup_ui();
    bool sanity_check();
    bool validate_delimiter(const QString& text);

    Ui::EditDocumentDialog *ui;
    QHash<QString, QString> document_type2id_;
    QVector<QString> document_types_;
    LEVEL level_;
    QString chip_id_, block_id_, register_id_, signal_id_;
    QString doc_id_;
    QVector<QString> signal_ids_;
    bool enabled_;
    DIALOG_MODE mode_;
    bool show_preview_;
    QString current_text_;
};


class DelegateLineEdit : public QStyledItemDelegate
{
public:
    DelegateLineEdit();
    ~DelegateLineEdit();
    void setCompleter(QCompleter* completer);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QCompleter *c;
};

#endif // DOCUMENT_EDITOR_H
