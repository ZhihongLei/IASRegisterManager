#ifndef EDIT_DOCUMENT_DIALOG_H
#define EDIT_DOCUMENT_DIALOG_H

#include <QDialog>
#include "data_utils.h"

namespace Ui {
    class EditDocumentDialog;
}

class EditDocumentDialog : public QDialog
{
    Q_OBJECT

public:
    enum DOCUMENT_LEVEL {BLOCK, SIGNAL};
    explicit EditDocumentDialog(DOCUMENT_LEVEL level, const QString& block_id = "", const QString& reg_id = "", const QString& sig_id = "", QWidget *parent = nullptr);
    explicit EditDocumentDialog(DOCUMENT_LEVEL level, const QString& doc_id, const QString& doc_type, const QString& content, const QString& block_id = "", const QString& reg_id = "", const QString& sig_id = "", bool enabled = true, QWidget *parent = nullptr);
    ~EditDocumentDialog();
    bool add_document();
    bool edit_document();

    QString get_document_type() const;
    QString get_document_type_id() const;
    QString get_doc_id() const;
    QString get_content() const;
    QString get_signal_id() const;


private slots:
    void on_comboBoxDocType_currentIndexChanged(int index);

    void on_textEditLaTeX_textChanged();

    void on_pushButtonSelectImage_clicked();

    void on_lineEditTableRow_editingFinished();

    void on_lineEditTableColumn_editingFinished();

    void on_pushButtonTablePreview_clicked();

    void on_tableWidget_cellClicked(int row, int column);
     void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_tableWidget_cellChanged(int row, int column);

private:
    void setup_ui();
    bool sanity_check();
    bool validate_table_cell_text(const QString& text);
    //bool validate_table_title(const QString& text);
    //bool validate_image_caption(const QString& text);

    void accept();
    Ui::EditDocumentDialog *ui;
    QHash<QString, QString> document_type2id_;
    QVector<QString> document_types_;
    const DOCUMENT_LEVEL level_;
    const QString block_id_, register_id_,signal_id_;
    QString doc_id_;
    QVector<QString> signal_ids_;
    const bool enabled_;
    const DIALOG_MODE mode_;
    bool show_preview_;
    QString current_text_;
};

#endif // EDIT_DOCUMENT_DIALOG_H
