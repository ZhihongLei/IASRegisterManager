#ifndef SPI_GENERATION_DIALOG_H
#define SPI_GENERATION_DIALOG_H

#include <QDialog>

namespace Ui {
class SPIGenerationDialog;
}

class SPIGenerationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SPIGenerationDialog(const QString& chip_id, const QString& chip_name,
                                 int register_width, int address_width,
                                 QWidget *parent = nullptr);
    ~SPIGenerationDialog();
    bool generate_spi_interface();

private slots:
    void on_pushButtonSelectConfig_clicked();
    void on_pushButtonSelectSPITemplate_clicked();
    void on_pushButtonSelectPackageTemplate_clicked();
    void on_comboBoxType_currentIndexChanged(int index);
    void on_pushButtonOutputSPI_clicked();
    void on_pushButtonOutputPkg_clicked();

private:
    void accept();
    bool sanity_check();
    bool check_paths();
    bool check_config();
    bool check_database();
    bool check_addresses(const QVector<QVector<QString> >& block_items);
    bool check_signal_partitions(const QVector<QVector<QString> >& block_items);
    bool check_register_partitions(const QVector<QVector<QString> >& block_items);

    Ui::SPIGenerationDialog *ui;
    const QString chip_id_, chip_name_;
    const int register_width_, address_width_;
};

#endif // SPI_GENERATION_DIALOG_H
