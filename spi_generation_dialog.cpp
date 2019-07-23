#include "spi_generation_dialog.h"
#include "ui_spi_generation_dialog.h"
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include "vhdl_generator.h"
#include <QMessageBox>
#include "database_handler.h"
#include "data_utils.h"
#include <QtMath>
#include "global_variables.h"

SPIGenerationDialog::SPIGenerationDialog(const QString& chip_id, const QString& chip_name,
                                         int register_width, int address_width, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SPIGenerationDialog),
    chip_id_(chip_id),
    chip_name_(chip_name),
    register_width_(register_width),
    address_width_(address_width)
{
    ui->setupUi(this);
    ui->comboBoxType->addItem("VHDL");
    setWindowTitle("SPI Generation");
}

SPIGenerationDialog::~SPIGenerationDialog()
{
    delete ui;
}

void SPIGenerationDialog::on_pushButtonSelectConfig_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Select Export Config", "", "INI files (*.ini)");
    if (path != "")
    {
        ui->lineEditConfig->setText(path);
        QSettings chip_setttings("chip_setttings.ini", QSettings::IniFormat);
        chip_setttings.beginGroup(chip_id_);
        chip_setttings.setValue("export_config_file", path);
        QSettings export_settings(ui->lineEditConfig->text(), QSettings::IniFormat);
        export_settings.beginGroup(ui->comboBoxType->currentText());
        if (ui->lineEditSPITemplate->text() != "") export_settings.setValue("spi_template_file", ui->lineEditSPITemplate->text());
        else ui->lineEditSPITemplate->setText(export_settings.value("spi_template_file").toString());
        if (ui->lineEditPackageTemplate->text() != "") export_settings.setValue("package_template_file", ui->lineEditPackageTemplate->text());
        else ui->lineEditPackageTemplate->setText(export_settings.value("package_template_file").toString());
        if (ui->lineEditOutputSPI->text() != "") export_settings.setValue("output_spi_file", ui->lineEditOutputSPI->text());
        else ui->lineEditOutputSPI->setText(export_settings.value("output_spi_file").toString());
        if (ui->lineEditOutputPkg->text() != "") export_settings.setValue("output_package_file", ui->lineEditOutputPkg->text());
        else ui->lineEditOutputPkg->setText(export_settings.value("output_package_file").toString());
    }
}

void SPIGenerationDialog::on_pushButtonSelectSPITemplate_clicked()
{
    QString filter;
    if (ui->comboBoxType->currentText() == "VHDL") filter = "VHDL (*.vhd)";
    QString path = QFileDialog::getOpenFileName(this, "Select SPI Template", "", filter);

    if (path != "")
    {
        ui->lineEditSPITemplate->setText(path);
        if (ui->lineEditConfig->text() != "")
        {
            QSettings export_settings(ui->lineEditConfig->text(), QSettings::IniFormat);
            export_settings.beginGroup(ui->comboBoxType->currentText());
            export_settings.setValue("spi_template_file", ui->lineEditSPITemplate->text());
        }
    }
}

void SPIGenerationDialog::on_pushButtonSelectPackageTemplate_clicked()
{
    QString filter;
    if (ui->comboBoxType->currentText() == "VHDL") filter = "VHDL (*.vhd)";
    QString path = QFileDialog::getOpenFileName(this, "Select Package Template", "", filter);

    if (path != "")
    {
        ui->lineEditPackageTemplate->setText(path);
        if (ui->lineEditConfig->text() != "")
        {
            QSettings export_settings(ui->lineEditConfig->text(), QSettings::IniFormat);
            export_settings.beginGroup(ui->comboBoxType->currentText());
            export_settings.setValue("package_template_file", ui->lineEditPackageTemplate->text());
        }
    }
}

void SPIGenerationDialog::on_pushButtonOutputSPI_clicked()
{
    QString filter;
    if (ui->comboBoxType->currentText() == "VHDL") filter = "VHDL (*.vhd)";
    QString path = QFileDialog::getSaveFileName(this, "SPI Save To", "", filter);
    if (path != "")
    {
        ui->lineEditOutputSPI->setText(path);
        if (ui->lineEditConfig->text() != "")
        {
            QSettings export_settings(ui->lineEditConfig->text(), QSettings::IniFormat);
            export_settings.beginGroup(ui->comboBoxType->currentText());
            export_settings.setValue("output_spi_file", ui->lineEditOutputSPI->text());
        }
    }
}

void SPIGenerationDialog::on_pushButtonOutputPkg_clicked()
{
    QString filter;
    if (ui->comboBoxType->currentText() == "VHDL") filter = "VHDL (*.vhd)";
    QString path = QFileDialog::getSaveFileName(this, "Package Save To", "", filter);
    if (path != "")
    {
        ui->lineEditOutputPkg->setText(path);
        if (ui->lineEditConfig->text() != "")
        {
            QSettings export_settings(ui->lineEditConfig->text(), QSettings::IniFormat);
            export_settings.beginGroup(ui->comboBoxType->currentText());
            export_settings.setValue("output_package_file", ui->lineEditOutputPkg->text());
        }
    }
}

void SPIGenerationDialog::on_comboBoxType_currentIndexChanged(int index)
{
    QSettings chip_setttings("chip_setttings.ini", QSettings::IniFormat);
    chip_setttings.beginGroup(chip_id_);
    ui->lineEditConfig->clear();
    ui->lineEditSPITemplate->clear();
    ui->lineEditPackageTemplate->clear();
    if (chip_setttings.value("export_config_file").toString() != "")
    {
        ui->lineEditConfig->setText(chip_setttings.value("export_config_file").toString());
        QSettings export_settings(ui->lineEditConfig->text(), QSettings::IniFormat);
        export_settings.beginGroup(ui->comboBoxType->currentText());
        ui->lineEditSPITemplate->setText(export_settings.value("spi_template_file").toString());
        ui->lineEditPackageTemplate->setText(export_settings.value("package_template_file").toString());
        ui->lineEditOutputSPI->setText(export_settings.value("output_spi_file").toString());
        ui->lineEditOutputPkg->setText(export_settings.value("output_package_file").toString());
    }
}

void SPIGenerationDialog::accept()
{
    if (sanity_check()) return QDialog::accept();
}

bool SPIGenerationDialog::sanity_check()
{
    return check_paths() && check_config() && check_database();
}

bool SPIGenerationDialog::check_paths()
{
    QString config_path = ui->lineEditConfig->text(),
            spi_interface_template_path = ui->lineEditSPITemplate->text(),
            interface_package_template_path = ui->lineEditPackageTemplate->text(),
            output_spi_path = ui->lineEditOutputSPI->text(),
            output_package_path = ui->lineEditOutputPkg->text();
    if (config_path == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify a generation config file!");
        return false;
    }
    if (spi_interface_template_path == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify an SPI interface template file!");
        return false;
    }
    if (interface_package_template_path == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify an interface package template file!");
        return false;
    }
    if (output_spi_path == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify an output SPI file!");
        return false;
    }
    if (output_package_path == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify an output package file!");
        return false;
    }
    if (!QFileInfo::exists(config_path))
    {
        QMessageBox::warning(this, windowTitle(), "Config file does not exist!");
        return false;
    }
    if (!QFileInfo::exists(spi_interface_template_path))
    {
        QMessageBox::warning(this, windowTitle(), "SPI interface template file does not exist!");
        return false;
    }
    if (!QFileInfo::exists(interface_package_template_path))
    {
        QMessageBox::warning(this, windowTitle(), "Interface package template file does not exist!");
        return false;
    }
    return true;
}

bool SPIGenerationDialog::check_config()
{
    QSettings config(ui->lineEditConfig->text(), QSettings::IniFormat);
    config.beginGroup(ui->comboBoxType->currentText());
    QString marker_addresses = config.value("marker_addresses").toString(),
            marker_package_inits = config.value("marker_package_inits").toString(),
            marker_ports = config.value("marker_ports").toString(),
            marker_register_definitions = config.value("marker_register_definitions").toString(),
            marker_register_init = config.value("marker_register_init").toString(),
            marker_register_write = config.value("marker_register_write").toString(),
            marker_register_read = config.value("marker_register_read").toString(),
            marker_signal_assignment = config.value("marker_signal_assignment").toString();
    if (marker_addresses.trimmed() == "" || marker_package_inits.trimmed() == "" || marker_ports.trimmed() == "" ||
            marker_register_definitions.trimmed() == "" || marker_register_init.trimmed() == "" ||
            marker_register_write.trimmed() == "" || marker_register_read.trimmed() == "" ||
            marker_signal_assignment.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please make sure markers in the config file are not empty!");
        return false;
    }

    QString address_suffix = config.value("address_suffix").toString(),
            init_suffix = config.value("init_suffix").toString(),
            global_reset_signal = config.value("global_reset_signal").toString(),
            read_buffer_signal = config.value("read_buffer_signal").toString(),
            reset_signal_suffix = config.value("reset_signal_suffix").toString(),
            spi_byte_signal = config.value("spi_byte_signal").toString(),
            varname_address_width = config.value("varname_address_width").toString(),
            varname_register_width = config.value("varname_register_width").toString();

    if (address_suffix.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify address_suffix in the config file!");
        return false;
    }
    if (init_suffix.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify init_suffix in the config file!");
        return false;
    }
    if (global_reset_signal.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify global_reset_signal in the config file!");
        return false;
    }
    if (read_buffer_signal.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify read_buffer_signal in the config file!");
        return false;
    }
    if (reset_signal_suffix.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify reset_signal_suffix in the config file!");
        return false;
    }
    if (spi_byte_signal.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify spi_byte_signal in the config file!");
        return false;
    }
    if (varname_address_width.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify varname_address_width in the config file!");
        return false;
    }
    if (varname_register_width.trimmed() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please specify varname_register_width in the config file!");
        return false;
    }
    return true;
}

bool SPIGenerationDialog::check_database()
{
    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items("block_system_block",
                                     {"block_id", "block_name", "abbreviation", "start_address", "prev", "next"},
                                     "chip_id", chip_id_, items))
    {
        QMessageBox::warning(this, windowTitle(),
                             "Unable to check system block addresses due to database connection error, Please try again!\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    return check_addresses(items) && check_signal_partitions(items) && check_register_partitions(items);
}

bool SPIGenerationDialog::check_addresses(const QVector<QVector<QString> >& block_items)
{
    QVector<QVector<QString> > start_addresses;
    for (const auto& block_item : block_items)
    {
        QString block_id = block_item[0],
                block_name = block_item[1],
                start_address = block_item[3];
        QVector<QString> count;
        DataBaseHandler::show_one_item("block_register", count, {"count(reg_id)"}, "block_id", block_id);
        start_addresses.push_back({start_address, count[0], block_name});
    }
    qSort(start_addresses.begin(),
          start_addresses.end(),
          [](const QVector<QString>&a, const QVector<QString>& b){return a[0].toULongLong(nullptr, 16) < b[0].toULongLong(nullptr, 16);});
    start_addresses.push_back({decimal2hex(quint64(qPow(2., address_width_) + 0.5), address_width_), "0", "end of address"});
    for (int i = 0; i < start_addresses.size() - 1; i++)
    {
        if (start_addresses[i][0].toULongLong(nullptr, 16) + start_addresses[i][1].toULongLong() > start_addresses[i+1][0].toULongLong(nullptr, 16))
        {
            QMessageBox::warning(this, windowTitle(),
                                 "Block " + start_addresses[i][2] + " contains too many registers and overlaps with " + start_addresses[i+1][2] + ".\nPlease make sure start addresses of the system blocks are correct!");
            return false;
        }
    }
    return true;
}

bool SPIGenerationDialog::check_signal_partitions(const QVector<QVector<QString> >& block_items)
{
    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING;
    for (const auto& block_item : block_items)
    {
        QVector<QVector<QString> > signal_items;
        QString block_id = block_item[0],
                block_name = block_item[1],
                block_abbr = block_item[2];
        signal_naming.update_key("${BLOCK_NAME}", block_name);
        signal_naming.update_key("${BLOCK_ABBR}", block_abbr);
        DataBaseHandler::show_items_inner_join({"signal_reg_signal.reg_sig_id", "signal_signal.sig_name", "signal_signal.width", "signal_signal.add_port"},
                                                {{{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}},
                                               signal_items, "block_id", block_id);

        for (const auto& signal_item: signal_items)
        {
            QString sig_name = signal_item[3] == "1" ? signal_naming.get_extended_name(signal_item[1]) : signal_item[1];
            QString reg_sig_id = signal_item[0],
                    signal_width = signal_item[2];
            QVector<QVector<QString> > part_items;
            DataBaseHandler::show_items("block_sig_reg_partition_mapping",
                                        {"sig_lsb", "sig_msb", "reg_lsb", "reg_msb"},
                                        "reg_sig_id", reg_sig_id, part_items);

            qSort(part_items.begin(), part_items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toInt() > b[1].toInt();});
            for (int i = 0; i < part_items.size(); i++)
            {
                const auto& part_item = part_items[i];
                QString sig_lsb = part_item[0],
                        sig_msb = part_item[1],
                        reg_lsb = part_item[2],
                        reg_msb = part_item[3];
                if (reg_msb.toInt() - reg_lsb.toInt() != sig_msb.toInt() - sig_lsb.toInt())
                {
                    QMessageBox::warning(this, windowTitle(), "Signal partition " + sig_name + "<" + sig_msb + ":" + sig_lsb +"> of block " + block_name + " does not match the register.\nPlease check it!");
                    return false;
                }
                if (i == 0 && sig_msb.toInt() >= signal_width.toInt())
                {
                    QMessageBox::warning(this, windowTitle(), "Partitions of signal " + sig_name + " of block " + block_name + " exceeds its signal width.\nPlease check it!");
                    return false;
                }
                if (i < part_items.size() - 1)
                {
                    const auto& next = part_items[i+1];
                    if (sig_lsb.toInt() <= next[1].toInt())
                    {
                        QMessageBox::warning(this, windowTitle(), "Partitions of signal " + sig_name + " of block " + block_name + " has overlaps.\nPlease check it!");
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool SPIGenerationDialog::check_register_partitions(const QVector<QVector<QString> >& block_items)
{
    NamingTemplate register_naming = GLOBAL_REGISTER_NAMING;
    for (const auto& block_item : block_items)
    {
        QVector<QVector<QString> > register_items;
        QString block_id = block_item[0],
                block_name = block_item[1],
                block_abbr = block_item[2];
        register_naming.update_key("${BLOCK_NAME}", block_name);
        register_naming.update_key("${BLOCK_ABBR}", block_abbr);

        DataBaseHandler::show_items("block_register", {"reg_id", "reg_name"}, "block_id", block_id, register_items);
        for (const auto& register_item: register_items)
        {
            QString reg_id = register_item[0],
                    reg_name = register_naming.get_extended_name(register_item[1]);
            QVector<QVector<QString> > part_items;
            DataBaseHandler::show_items("block_sig_reg_partition_mapping",
                                        {"sig_lsb", "sig_msb", "reg_lsb", "reg_msb"},
                                        "reg_id", reg_id, part_items);

            qSort(part_items.begin(), part_items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[3].toInt() > b[3].toInt();});
            for (int i = 0; i < part_items.size(); i++)
            {
                const auto& part_item = part_items[i];
                QString sig_lsb = part_item[0],
                        sig_msb = part_item[1],
                        reg_lsb = part_item[2],
                        reg_msb = part_item[3];
                if (reg_msb.toInt() - reg_lsb.toInt() != sig_msb.toInt() - sig_lsb.toInt())
                {
                    QMessageBox::warning(this, windowTitle(), "Register partition " + reg_name + "<" + reg_msb + ":" + reg_lsb +"> of block " + block_name + " does not match the signal.\nPlease check it!");
                    return false;
                }
                if (i == 0 && reg_msb.toInt() >= register_width_)
                {
                    QMessageBox::warning(this, windowTitle(), "Partitions of register " + reg_name + " of block " + block_name + " exceeds register width.\nPlease check it!");
                    return false;
                }
                if (i < part_items.size() - 1)
                {
                    const auto& next = part_items[i+1];
                    if (reg_lsb.toInt() <= next[3].toInt())
                    {
                        QMessageBox::warning(this, windowTitle(), "Partitions of register " + reg_name + " of block " + block_name + " has overlaps.\nPlease check it!");
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool SPIGenerationDialog::generate_spi_interface()
{
    QString interface_package_path, spi_interface_path;
    QString interface_package, spi_interface;
    if (ui->comboBoxType->currentText() == "VHDL")
    {
        VHDLGenerator generator(chip_id_, chip_name_, address_width_, register_width_,
                          ui->lineEditConfig->text(), ui->lineEditSPITemplate->text(), ui->lineEditPackageTemplate->text());
        interface_package = generator.generate_interface_package();
        spi_interface = generator.generate_interface();
        interface_package_path = ui->lineEditOutputPkg->text();
        spi_interface_path = ui->lineEditOutputSPI->text();
        if (!generator.success())
        {
            QMessageBox::warning(this, windowTitle(), "Unable to generate SPI interface due to database connection error.\nError message: " + DataBaseHandler::get_error_message() + "!");
            return false;
        }
    }

    QFile interface_package_file(interface_package_path);
    QFile spi_interface_file(spi_interface_path);
    if( !interface_package_file.open(QIODevice::WriteOnly) || !spi_interface_file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to generate SPI interface due to IO error.\nPlease try again!");
        interface_package_file.close();
        spi_interface_file.close();
        return false;
    }

    QTextStream output_interface_package(&interface_package_file);
    QTextStream output_spi_interface(&spi_interface_file);
    output_interface_package << interface_package;
    output_spi_interface << spi_interface;
    interface_package_file.close();
    spi_interface_file.close();
    return true;
}
