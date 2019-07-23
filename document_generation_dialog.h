#ifndef DOCUMENT_GENERATION_DIALOG_H
#define DOCUMENT_GENERATION_DIALOG_H

#include <QDialog>

class QListWidgetItem;
class Authenticator;
namespace Ui {
class DocumentGenerationDialog;
}

class DocumentGenerationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DocumentGenerationDialog(const QString& chip_id,
                                      const QString& chip_name,
                                      int register_width,
                                      int address_width,
                                      bool msb_first,
                                      Authenticator* authenticator,
                                      const QString& user_id,
                                      QWidget *parent = nullptr);
    ~DocumentGenerationDialog();
    QString get_register_naming() const;
    QString get_signal_naming() const;
    QString get_image_caption_position() const;
    QString get_table_caption_position() const;
    QString get_show_paged_register() const;
    bool generate_document();

private slots:
    void on_comboBoxDocType_currentIndexChanged(int index);
    void on_pushButtonRegNaming_clicked();
    void on_pushButtonSigNaming_clicked();
    void on_listWidgetToExport_itemClicked(QListWidgetItem *item);
    void on_checkBoxSelectAll_toggled(bool checked);
    void on_pushButtonSelectPath_clicked();

private:
    void accept();
    bool sanity_check();
    Ui::DocumentGenerationDialog *ui;
    const QString chip_id_, chip_name_, user_id_;
    int register_width_, address_width_;
    bool msb_first_;
    Authenticator* authenticator_;
    QVector<QString> block_ids_, block_names_, block_abbrs_, block_start_addrs_;
    QVector<Qt::CheckState> check_states_;
};

#endif // DOCUMENT_GENERATION_DIALOG_H
