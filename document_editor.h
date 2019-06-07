#ifndef DOCUMENT_EDITOR_H
#define DOCUMENT_EDITOR_H

#include <QWidget>
#include "data_utils.h"

namespace Ui {
class DocumentEditor;
}

class DocumentEditor : public QWidget
{
    Q_OBJECT

public:
    explicit DocumentEditor(QWidget *parent = nullptr);
    ~DocumentEditor();
    enum DOCUMENT_LEVEL {BLOCK, REGISTER, SIGNAL};
    bool add_document();
    bool edit_document();

    QString get_document_type() const;
    QString get_document_type_id() const;
    QString get_doc_id() const;
    QString get_content() const;
    QString get_signal_id() const;
    void set_content(const QString& doc_id, const QString& doc_type, const QString& content);
    void set_mode(const DIALOG_MODE& mode);
    void clear();
    void set_block_id(const QString& block_id);
    void set_register_id(const QString& reg_id);
    void set_signal_id(const QString& sig_id);
    void set_level(const DOCUMENT_LEVEL& level);
    void setEnabled(bool enabled);

signals:
    void document_added();
    void document_edited();

private slots:
    void on_comboBoxDocType_currentIndexChanged(int index);

    void on_textEditText_textChanged();

    void on_pushButtonSelectImage_clicked();

    void on_lineEditImageCaption_editingFinished();

    void on_lineEditTableRow_editingFinished();

    void on_lineEditTableColumn_editingFinished();

    void on_pushButtonPreview_clicked();

    void on_tableWidget_cellClicked(int row, int column);
    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_tableWidget_cellChanged(int row, int column);

    void on_lineEditTableCaption_editingFinished();

    //void on_add_document();
    //void on_edit_document();

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    QString generate_html_table();

private:
    void setup_ui();
    bool sanity_check();
    bool validate_table_cell_text(const QString& text);

    //bool validate_table_title(const QString& text);
    //bool validate_image_caption(const QString& text);

    void accept();
    Ui::DocumentEditor *ui;
    QHash<QString, QString> document_type2id_;
    QVector<QString> document_types_;
    DOCUMENT_LEVEL level_;
    QString block_id_, register_id_, signal_id_;
    QString doc_id_;
    QVector<QString> signal_ids_;
    bool enabled_;
    DIALOG_MODE mode_;
    bool show_preview_;
    QString current_text_;
};

#endif // DOCUMENT_EDITOR_H
