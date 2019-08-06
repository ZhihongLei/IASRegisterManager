#include "vhdl_generator.h"
#include "global_variables.h"
#include "database_handler.h"
#include "data_utils.h"
#include <QSettings>
#include <QFile>
#include <QMessageBox>
#include <QDebug>

bool VHDLGenerator::success_;

VHDLGenerator::VHDLGenerator(const QString& chip_id,
              const QString& chip_name,
              int address_width,
              int register_width,
              const QString& config_file,
              const QString& interface_template_path,
              const QString& interface_package_template_path):
    chip_id_(chip_id),
    chip_name_(chip_name),
    address_width_(address_width),
    register_width_(register_width),
    interface_template_path_(interface_template_path),
    interface_package_template_path_(interface_package_template_path)
{
    config_ = new QSettings(config_file, QSettings::IniFormat);
    config_->beginGroup("VHDL");
    reset_success_flag();
}

VHDLGenerator::~VHDLGenerator()
{
    if (config_) delete config_;
}

void VHDLGenerator::reset_success_flag()
{
    success_ = true;
}

bool VHDLGenerator::success()
{
    return success_;
}

QString VHDLGenerator::generate_interface_package() const
{
    QVector<QVector<QString> > items;

    auto rw_types = get_register_rw_types();
    QSet<QString> readable_reg_types = rw_types.first,
                  writable_reg_types = rw_types.second;
    success_ = success_ && DataBaseHandler::show_items("block_system_block",
                                {"block_id", "block_name", "abbreviation", "start_address", "prev", "next"},
                                "chip_id", chip_id_, items);
    sort_doubly_linked_list(items);
    QString addresses, inits;

    for (const auto& item : items)
    {
        QString block_id= item[0], block_name = item[1], block_abbr = item[2], block_start_addr = item[3];
        QPair<QString, QString> p = generate_interface_package_block(block_id, block_name, block_abbr, block_start_addr, writable_reg_types);
        addresses += p.first;
        inits += p.second;
    }

    QFile file(interface_package_template_path_);
    if( !file.open(QIODevice::ReadOnly) )
    {
      QMessageBox::warning(nullptr, "VHDL Generation", "Unable to read interface package template due to IO error.\nPlease try again!");
      return "";
    }
    QTextStream instream(&file);
    QString interface_package = instream.readAll();
    QString marker_addresses = config_->value("marker_addresses").toString(),
            marker_package_inits = config_->value("marker_package_inits").toString();
    interface_package.replace(marker_addresses, addresses).replace(marker_package_inits, inits);
    return interface_package;
}

QString VHDLGenerator::generate_interface() const
{
    QString ports,
            register_definitions, register_inits,
            register_write_accesses, register_read_accesses,
            readonly_register_assignments, control_signal_assignments, paged_register_assignments;

    auto rw_types = get_register_rw_types();
    QSet<QString> readable_reg_types = rw_types.first,
                  writable_reg_types = rw_types.second;
    auto inout_types = get_signal_inout_types(readable_reg_types, writable_reg_types);
    QSet<QString> input_signal_types = inout_types.first,
                  output_signal_types = inout_types.second;

    QHash<QString, QVector<QString> > reg_id2page = get_register_id2page();

    QVector<QVector<QString> > items;
    success_ = success_ && DataBaseHandler::show_items("block_system_block",
                                {"block_id", "block_name", "abbreviation", "start_address", "prev", "next"},
                                "chip_id", chip_id_, items);
    sort_doubly_linked_list(items);
    for (const auto& item : items)
    {
        QString block_id= item[0], block_name = item[1], block_abbr = item[2], block_start_addr = item[3];
        QVector<QString> interface_block = generate_interface_block(block_id, block_name, block_abbr, writable_reg_types, output_signal_types, reg_id2page);
        ports += interface_block[0];
        register_definitions += interface_block[1];
        register_inits += interface_block[2];
        register_write_accesses += interface_block[3];
        register_read_accesses += interface_block[4];
        readonly_register_assignments += interface_block[5];
        control_signal_assignments += interface_block[6];
        paged_register_assignments += interface_block[7];
    }
    QFile file(interface_template_path_);
    if( !file.open(QIODevice::ReadOnly) )
    {
      QMessageBox::warning(nullptr, "VHDL Generation", "Unable to read interface template due to IO error.\nPlease try again!");
      return "";
    }
    QTextStream instream(&file);
    QString interface = instream.readAll();
    QString marker_ports = config_->value("marker_ports").toString(),
            marker_register_definitions = config_->value("marker_register_definitions").toString(),
            marker_register_init = config_->value("marker_register_init").toString(),
            marker_register_write = config_->value("marker_register_write").toString(),
            marker_register_read = config_->value("marker_register_read").toString(),
            marker_signal_assignment = config_->value("marker_signal_assignment").toString();

    QString signal_assignments = "-- READ ONLY SIGNAL ASSIGNMENTS\n" + readonly_register_assignments +
            "\n  -- READ AND WRITE ASSIGNMENTS\n" + control_signal_assignments +
            "\n  -- PAGED R/W REGISTER SIGNAL MUXING\n" + paged_register_assignments;

    int idx = ports.lastIndexOf(";\n");
    if (idx != -1) ports.remove(idx, 1);
    interface.replace(marker_ports, ports.remove(QRegExp("^[ ]*"))).replace(
                marker_register_definitions, register_definitions.remove(QRegExp("^[ ]*"))).replace(
                marker_register_init, register_inits.remove(QRegExp("^[ ]*"))).replace(
                marker_register_write, register_write_accesses.remove(QRegExp("^[ ]*"))).replace(
                marker_register_read, register_read_accesses.remove(QRegExp("^[ ]*"))).replace(
                marker_signal_assignment, signal_assignments.remove(QRegExp("^[ ]*")));
    return interface;
}

QPair<QString, QString> VHDLGenerator::generate_interface_package_block(const QString &block_id,
                                                        const QString &block_name,
                                                        const QString &block_abbr,
                                                        const QString &block_start_addr,
                                                        const QSet<QString>& writable_reg_types) const
{
    QString addresses, inits;
    quint64 block_start_address_decimal = block_start_addr.toULongLong(nullptr, 16);
    QString addr_suffix = config_->value("address_suffix").toString(),
            init_suffix = config_->value("init_suffix").toString();
    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING,
            register_naming = GLOBAL_REGISTER_NAMING;

    register_naming.update_key("${BLOCK_NAME}", block_name);
    register_naming.update_key("${BLOCK_ABBR}", block_abbr);
    signal_naming.update_key("${BLOCK_NAME}", block_name);
    signal_naming.update_key("${BLOCK_ABBR}", block_abbr);

    QString headline = "  --" + block_abbr + "\n";

    QVector<QVector<QString> > registers;
    success_ = success_ && DataBaseHandler::show_items("block_register", {"reg_id", "reg_name", "reg_type_id", "prev", "next"}, "block_id", block_id, registers);
    registers = sort_doubly_linked_list(registers);

    for (int i = 0; i < registers.size(); i++)
    {
        const auto& reg = registers[i];
        QString reg_id = reg[0],
                reg_name = register_naming.get_extended_name(reg[1]),
                reg_type_id = reg[2];
        QString addr = decimal2hex(block_start_address_decimal + static_cast<quint64>(i), address_width_);
        addresses += generate_register_address_definition(reg_name, addr);
        if (writable_reg_types.contains(reg_type_id))
            inits += generate_register_init_definition(reg_id, reg_name, signal_naming);
    }
    addresses = headline + addresses;
    inits = headline + inits;
    return {addresses, inits};
}

QVector<QString> VHDLGenerator::generate_interface_block(const QString &block_id,
                                                const QString &block_name,
                                                const QString &block_abbr,
                                                const QSet<QString> &writable_reg_types,
                                                const QSet<QString>& output_signal_types,
                                                const QHash<QString, QVector<QString> >& reg_id2page) const
{
    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING,
            register_naming = GLOBAL_REGISTER_NAMING;

    register_naming.update_key("${BLOCK_NAME}", block_name);
    register_naming.update_key("${BLOCK_ABBR}", block_abbr);
    signal_naming.update_key("${BLOCK_NAME}", block_name);
    signal_naming.update_key("${BLOCK_ABBR}", block_abbr);

    QString headline = "--" + block_abbr + "\n";
    QVector<QVector<QString> > register_items, signal_items;
    success_ = success_ && DataBaseHandler::show_items("block_register", {"reg_id", "reg_name", "reg_type_id", "prev", "next"}, "block_id", block_id, register_items);
    register_items = sort_doubly_linked_list(register_items);

    QString ports,
            register_definitions, register_inits,
            register_write_accesses, register_read_accesses,
            readonly_register_assignments, control_signal_assignments, paged_register_assignments;
    for (int i = 0; i < register_items.size(); i++)
    {
        const auto& reg = register_items[i];
        QString reg_id = reg[0],
                reg_name = register_naming.get_extended_name(reg[1]),
                reg_type_id = reg[2];
        register_definitions += generate_register_definition(reg_id, reg_name, reg_id2page);
        register_read_accesses += generate_reading_register(reg_name);
        if (writable_reg_types.contains(reg_type_id))
        {
            register_inits += generate_initializing_register(reg_id, reg_name, reg_id2page);
            register_write_accesses += generate_writing_register(reg_id, reg_name, reg_id2page);
        }
        else
        {
            readonly_register_assignments = generate_assigning_value_to_readonly_register(reg_id, reg_name, signal_naming);
        }
        if (reg_id2page.contains(reg_id))
            paged_register_assignments += generate_assigning_value_to_paged_register(reg_id, reg_name, reg_id2page);
    }

    QString reset_signal_suffix = config_->value("reset_signal_suffix").toString();
    success_ = success_ && DataBaseHandler::show_items("signal_signal", {"sig_id", "sig_name", "width", "sig_type_id", "add_port"}, "block_id", block_id, signal_items);
    for (int i = 0; i < signal_items.size(); i++)
    {
        QString sig_id = signal_items[i][0],
                sig_name = signal_items[i][1],
                sig_type_id = signal_items[i][3],
                add_port = signal_items[i][4];
        bool reset_signal = sig_name.right(reset_signal_suffix.size()) == reset_signal_suffix;
        int sig_width = signal_items[i][2].toInt();
        if (add_port == "1")
        {
            sig_name = signal_naming.get_extended_name(sig_name);
            ports += generate_port_definition(sig_name, sig_width, sig_type_id, output_signal_types);
        }
        if (output_signal_types.contains(sig_type_id))
            control_signal_assignments += generate_assigning_value_to_constrol_signal(sig_id, sig_name, sig_width, reset_signal, register_naming);
    }
    return {"    " + headline + ports,
            "  " + headline + register_definitions,
            "      " + headline + register_inits,
            QString(12, ' ')  + headline + register_write_accesses,
            QString(12, ' ')  + headline + register_read_accesses,
            "  " + headline + readonly_register_assignments,
            "  " +  headline + control_signal_assignments,
            "  " +  headline + paged_register_assignments};
}

QString VHDLGenerator::generate_register_address_definition(const QString &reg_name,
                                                            const QString& reg_address) const
{
    QString reg_addr_name = reg_name + config_->value("address_suffix").toString();
    if (reg_addr_name.size() < 40) reg_addr_name = reg_addr_name + QString(40 - reg_addr_name.size(), ' ');
    QString logic = QString("std_logic_vector(${ADDR_WIDTH_VAR} - 1 downto 0)").replace("${ADDR_WIDTH_VAR}", config_->value("varname_address_width").toString());
    QString address = "x\"" + reg_address + "\"";
    return "  constant " + reg_addr_name + " : " + logic + " := " + address + ";\n";
}

QString VHDLGenerator::generate_register_init_definition(const QString &reg_id,
                                                         const QString &reg_name,
                                                         const NamingTemplate& signal_naming) const
{
    QString reg_init_name = reg_name + config_->value("init_suffix").toString();
    if (reg_init_name.size() < 40) reg_init_name = reg_init_name + QString(40 - reg_init_name.size(), ' ');
    QString logic = QString("std_logic_vector(${REG_WIDTH_VAR} - 1 downto 0)").replace("${REG_WIDTH_VAR}", config_->value("varname_register_width").toString());
    QVector<QChar> bits;
    QVector<QString> signal_parts;

    QVector<QVector<QString> > items;
    QVector<QString> extended_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                       "block_sig_reg_partition_mapping.reg_lsb",
                                       "block_sig_reg_partition_mapping.reg_msb",
                                       "signal_signal.sig_name",
                                       "signal_signal.sig_id",
                                       "block_sig_reg_partition_mapping.sig_lsb",
                                       "block_sig_reg_partition_mapping.sig_msb",
                                        "signal_reg_signal.init_value",
                                        "signal_signal.width",
                                        "signal_signal.add_port"};
    success_ = success_ && DataBaseHandler::show_items_inner_join(extended_fields, {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                 {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, items, {{"block_sig_reg_partition_mapping.reg_id", reg_id}});

    qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() > b[2].toInt();});

    if (items.size() == 0) {
        for (int j = 0; j < register_width_; j++)
        {
            bits.push_back('0');
            signal_parts.push_back("*");
        }
    }
    else {
        for (int j = 0; j < items.size(); j++)
        {
            const auto &curr = items[j];
            QString reg_msb = curr[2],
                    reg_lsb = curr[1],
                    sig_msb = curr[6],
                    sig_lsb = curr[5],
                    sig_name = curr[9] == "1" ? signal_naming.get_extended_name(curr[3]) : curr[3],
                    sig_value = QString::number(curr[7].toULongLong(nullptr, 16), 2);
            int sig_width = curr[8].toInt();
            sig_value = QString(sig_width - sig_value.size(), '0') + sig_value;
            signal_parts.push_back(sig_width > 1 ? sig_name + "<" + sig_msb + ":" + sig_lsb + ">" : sig_name);

            for (int bit = sig_msb.toInt(); bit >= sig_lsb.toInt(); bit--) bits.push_back(sig_value[sig_width-bit-1]);
            if (j == 0)
            {
                int gap = register_width_ - reg_msb.toInt() -1 ;
                while (gap > 0)
                {
                    bits.insert(0, '0');
                    signal_parts.insert(0, "*");
                    gap--;
                }
            }

            int gap = reg_lsb.toInt();
            if (j != items.size() - 1)
            {
                const auto &next = items[j+1];
                gap = gap - next[2].toInt() - 1;
            }
            while (gap > 0)
            {
                bits.push_back('0');
                signal_parts.push_back("*");
                gap--;
            }
        }
    }
    QString init_value, comment;
    for (const QChar& bit : bits)
        init_value += bit;
    for (const QString& sig_part : signal_parts)
        comment += sig_part + " | ";
    comment = "| " + comment;
    return "  constant " + reg_init_name + " : " + logic + " := \"" + init_value + "\"; -- " + comment + "\n";
}

QString VHDLGenerator::generate_port_definition(const QString &sig_name,
                                                int sig_width,
                                                const QString &sig_type_id,
                                                const QSet<QString> &output_signal_types) const
{
    QString padded_sig_name = sig_name.size() < 40 ? sig_name + QString(40 - sig_name.size(), ' ') : sig_name;
    QString direction = output_signal_types.contains(sig_type_id) ? "out" : "in";
    QString logic =  sig_width == 1 ?  "std_logic" :
                                    QString("std_logic_vector(${SIGNAL_WIDTH} - 1 downto 0)").replace("${SIGNAL_WIDTH}", QString::number(sig_width));
    return "    " + padded_sig_name + " : " + direction + "  " + logic + ";\n";
}

QString VHDLGenerator::generate_register_definition(const QString& reg_id,
                                                    const QString &reg_name,
                                                    const QHash<QString, QVector<QString> > &reg_id2page) const
{
    QString definition;
    QString logic = QString("std_logic_vector(${REG_WIDTH_VAR} - 1 downto 0)").replace("${REG_WIDTH_VAR}", config_->value("varname_register_width").toString());
    QString padded_reg_name = reg_name.size() < 40 ? reg_name + QString(40 - reg_name.size(), ' ') : reg_name;
    definition += "  signal " + padded_reg_name + " : " + logic + ";\n";

    if (reg_id2page.contains(reg_id))
    {
        QString page_name = reg_id2page[reg_id][0];
        int page_count = reg_id2page[reg_id][1].toInt();
        for (int i = 0; i < page_count; i++)
        {
            QString padded_reg_name = reg_name + "_" + page_name + "_" + QString::number(i);
            if (padded_reg_name.size() < 40) padded_reg_name += QString(40 - padded_reg_name.size(), ' ');
            definition += "  signal " + padded_reg_name + " : " + logic + ";\n";
        }
    }
    return definition;
}

QString VHDLGenerator::generate_initializing_register(const QString &reg_id, const QString &reg_name, const QHash<QString, QVector<QString> > &reg_id2page) const
{
    QString init;
    QString reg_init_name = reg_name + config_->value("init_suffix").toString();
    if (reg_id2page.contains(reg_id))
    {
        QString page_name = reg_id2page[reg_id][0];
        int page_count = reg_id2page[reg_id][1].toInt();
        for (int i = 0; i < page_count; i++)
        {
            QString padded_reg_name = reg_name + "_" + page_name + "_" + QString::number(i);
            if (padded_reg_name.size() < 40) padded_reg_name += QString(40 - padded_reg_name.size(), ' ');
            init += "      " + padded_reg_name + " <= " + reg_init_name + ";\n";
        }
    }
    else
    {
        QString padded_reg_name = reg_name.size() < 40 ? reg_name + QString(40 - reg_name.size(), ' ') : reg_name;
        init += "      " + padded_reg_name + " <= " + reg_init_name + ";\n";
    }
    return init;
}

QString VHDLGenerator::generate_writing_register(const QString &reg_id,
                                                 const QString &reg_name,
                                                 const QHash<QString, QVector<QString> > &reg_id2page) const
{
    QString reg_addr_name = reg_name + config_->value("address_suffix").toString();
    if (reg_addr_name.size() < 40) reg_addr_name = reg_addr_name + QString(40 - reg_addr_name.size(), ' ');
    QString spi_byte_signal = config_->value("spi_byte_signal").toString();

    QString statement;
    if (reg_id2page.contains(reg_id))
    {
        QString page_name = reg_id2page[reg_id][0], sig_name = reg_id2page[reg_id][2];
        int page_count = reg_id2page[reg_id][1].toInt(), sig_width = reg_id2page[reg_id][3].toInt();
        statement = "case " + sig_name + " is";
        for (int i = 0; i < page_count; i++)
        {
            QString paged_reg_name = reg_name + "_" + page_name + "_" + QString::number(i);
            QString sig_value;
            if (i < page_count - 1)
            {
                sig_value = QString::number(i, 2);
                sig_value = QString(sig_width - sig_value.size(), '0') + sig_value;
                sig_value = sig_width > 1 ? "\"" + sig_value + "\"" : "\'" + sig_value + "\'";
            }
            else sig_value = "others";
            statement += " when " +  sig_value + " => " + paged_reg_name + " <= " + spi_byte_signal + ";";
        }
        statement += " end case ;";
    }
    else
    {
        QString padded_reg_name = reg_name.size() < 40 ? reg_name + QString(40 - reg_name.size(), ' ') : reg_name;
        statement = padded_reg_name + " <= " + spi_byte_signal + ";";
    }
    return QString(12, ' ') + "when " + reg_addr_name + " => " + statement + "\n";
}

QString VHDLGenerator::generate_reading_register(const QString &reg_name) const
{
    QString reg_addr_name = reg_name + config_->value("address_suffix").toString();
    if (reg_addr_name.size() < 40) reg_addr_name = reg_addr_name + QString(40 - reg_addr_name.size(), ' ');
    QString read_buffer_signal = config_->value("read_buffer_signal").toString();
    if (read_buffer_signal.size() < 40) read_buffer_signal = read_buffer_signal + QString(40 - read_buffer_signal.size(), ' ');

    return QString(12, ' ') + "when " + reg_addr_name + " => " + read_buffer_signal + " <= " + reg_name +"\n";
}

QString VHDLGenerator::generate_assigning_value_to_readonly_register(const QString &reg_id, const QString &reg_name, const NamingTemplate& signal_naming) const
{
    QString padded_reg_name = reg_name.size() < 40 ? reg_name + QString(40 - reg_name.size(), ' ') : reg_name;
    QVector<QString> values;
    QVector<QVector<QString> > items;

    QVector<QString> extended_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                       "block_sig_reg_partition_mapping.reg_lsb",
                                       "block_sig_reg_partition_mapping.reg_msb",
                                       "signal_signal.sig_name",
                                       "block_sig_reg_partition_mapping.sig_lsb",
                                       "block_sig_reg_partition_mapping.sig_msb",
                                        "signal_signal.width",
                                        "signal_signal.add_port"};
    success_ = success_ && DataBaseHandler::show_items_inner_join(extended_fields, {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                 {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, items, {{"block_sig_reg_partition_mapping.reg_id", reg_id}});
    qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() > b[2].toInt();});

    if (items.size() == 0)
    {
        values.push_back( "\"" + QString(register_width_, '0') + "\"");
    }
    for (int i = 0; i < items.size(); i++)
    {
        const auto& item = items[i];
        int reg_lsb = item[1].toInt(),
            reg_msb = item[2].toInt(),
            sig_width = item[6].toInt();
        QString sig_lsb = item[4],
                sig_msb = item[5];

        QString sig_name = item[3];
        if (item[7] == "1") sig_name = signal_naming.get_extended_name(sig_name);

        if (i == 0 && reg_msb < register_width_ - 1) values.insert(0, "\"" + QString(register_width_ - reg_msb - 1, '0') + "\"");
        if (sig_width == 1) values.push_back(sig_name);
        else if (sig_msb != sig_lsb) values.push_back(sig_name + QString("(${MSB} downto ${LSB})").replace("${MSB}", sig_msb).replace("${LSB}", sig_lsb));
        else values.push_back(sig_name + '(' + sig_lsb + ')');
        if (i == items.size() - 1 && reg_lsb > 0) values.push_back("\"" + QString(reg_lsb, '0') + "\"");
    }

    QString statement = "  " + padded_reg_name + " <= ";
    for (int i = 0; i < values.size(); i++)
        statement += (i != values.size() - 1 ? values[i] + " & " : values[i] + ";\n");
    return statement;

}

QString VHDLGenerator::generate_assigning_value_to_constrol_signal(const QString &sig_id, const QString &sig_name, int sig_width, bool reset_signal, const NamingTemplate& register_naming) const
{
    QString padded_sig_name = sig_name.size() < 40 ? sig_name + QString(40 - sig_name.size(), ' ') : sig_name;
    QVector<QVector<QString> > items;
    QVector<QString> values;
    QString global_reset_signal = config_->value("global_reset_signal").toString();

    QVector<QString> extended_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                           "block_sig_reg_partition_mapping.sig_lsb",
                                           "block_sig_reg_partition_mapping.sig_msb",
                                           "block_register.reg_name",
                                           "block_sig_reg_partition_mapping.reg_lsb",
                                           "block_sig_reg_partition_mapping.reg_msb"};
    success_ = success_ && DataBaseHandler::show_items_inner_join(extended_fields,
                                              {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}},
                                                {{"block_sig_reg_partition_mapping", "reg_id"}, {"block_register", "reg_id"}}},
                                               items, {{"signal_signal.sig_id", sig_id}});
    qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() > b[2].toInt();});
    if (items.size() == 0)
    {
        values.push_back(sig_width > 1 ? "\"" + QString(sig_width, '0') + "\"" : "\'0\'");
    }
    for (int i = 0; i < items.size(); i++)
    {
        const auto& item = items[i];
        int sig_lsb = item[1].toInt(),
            sig_msb = item[2].toInt();
        QString reg_lsb = item[4],
                reg_msb = item[5],
                reg_name = register_naming.get_extended_name(item[3]);

        if (i == 0 && sig_msb < sig_width - 1) values.insert(0, "\"" + QString(sig_width - sig_msb - 1, '0') + "\"");
        if (sig_msb != sig_lsb) values.push_back(reg_name + QString("(${MSB} downto ${LSB})").replace("${MSB}", reg_msb).replace("${LSB}", reg_lsb));
        else values.push_back(reg_name + '(' + reg_lsb + ')');
        if (i == items.size() - 1 && sig_lsb > 0) values.push_back("\"" + QString(sig_lsb, '0') + "\"");
    }

    QString statement;
    for (int i = 0; i < values.size(); i++)
        statement += (i != values.size() - 1 ? values[i] + " & " : values[i]);
    if (reset_signal)
    {
        if (values.size() > 1) statement = "(" + statement + ")";
        statement += " when " + global_reset_signal + " = \'1\' else " + (sig_width == 1 ? "\'0\'" : "\"" + QString(sig_width, '0') + "\"");
    }
    statement = "  " + padded_sig_name + " <= " + statement + ";\n";
    return statement;
}

QString VHDLGenerator::generate_assigning_value_to_paged_register(const QString &reg_id, const QString &reg_name, const QHash<QString, QVector<QString> >& reg_id2page) const
{
    QString padded_reg_name = reg_name.size() < 40 ? reg_name + QString(40 - reg_name.size(), ' ') : reg_name;
    QString page_name = reg_id2page[reg_id][0],
            sig_name = reg_id2page[reg_id][2];
    int page_count = reg_id2page[reg_id][1].toInt(),
        sig_width = reg_id2page[reg_id][3].toInt();

    QVector<QString> values;
    for (int i = 0; i < page_count; i++)
    {
        QString paged_reg_name = reg_name + "_" + page_name + "_" + QString::number(i);
        QString value;
        if (i < page_count - 1)
        {
            QString sig_value;
            sig_value = QString::number(i, 2);
            sig_value = QString(sig_width - sig_value.size(), '0') + sig_value;
            sig_value = sig_width > 1 ? "\"" + sig_value + "\"" : "\'" + sig_value + "\'";
            QString condition = " when (" + sig_name + " = " + sig_value + ")";
            value = paged_reg_name + condition;
        }
        else value = paged_reg_name;
        values.push_back(value);
    }

    QString statement = "  " + padded_reg_name + " <= ";
    for (int i = 0; i < values.size(); i++)
        statement += (i != values.size() - 1 ? values[i] + " else " : values[i] + ";\n");
    return statement;
}

QPair<QSet<QString>, QSet<QString> > VHDLGenerator::get_register_rw_types() const
{
    QVector<QVector<QString> > items;
    success_ = success_ && DataBaseHandler::show_items("def_register_type", {"reg_type_id", "readable", "writable"}, items);
    QSet<QString> readable_reg_types, writable_reg_types;
    for (const auto& item: items)
    {
        if (item[1] == "1") readable_reg_types.insert(item[0]);
        if (item[2] == "1") writable_reg_types.insert(item[0]);
    }
    return {readable_reg_types, writable_reg_types};
}

QPair<QSet<QString>, QSet<QString> > VHDLGenerator::get_signal_inout_types(const QSet<QString> &readable_reg_types,
                                                                           const QSet<QString> &writable_reg_types) const
{
    QVector<QVector<QString> > items;
    QSet<QString> input_signal_types, output_signal_types;
    success_ = success_ && DataBaseHandler::show_items("def_sig_reg_type_mapping", {"sig_type_id", "reg_type_id"}, items);
    for (const auto& item: items)
    {
        if (writable_reg_types.contains(item[1])) output_signal_types.insert(item[0]);
        else if (readable_reg_types.contains(item[1])) input_signal_types.insert(item[0]);
    }
    return {input_signal_types, output_signal_types};
}

QHash<QString, QVector<QString> > VHDLGenerator::get_register_id2page() const
{
    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING;
    QVector<QVector<QString> > items;
    QHash<QString, QVector<QString> > reg_id2page;
    success_ = success_ && DataBaseHandler::show_items_inner_join({"chip_register_page.page_id",
                                            "chip_register_page.page_name",
                                            "signal_signal.sig_name",
                                            "signal_signal.width",
                                            "chip_register_page.page_count",
                                            "signal_signal.add_port",
                                            "block_system_block.block_name",
                                            "block_system_block.abbreviation"
                                            },
                                            {{{"chip_register_page", "ctrl_sig"}, {"signal_signal", "sig_id"}},
                                             {{"signal_signal", "block_id"}, {"block_system_block", "block_id"}}},
                                           items, "chip_register_page.chip_id", chip_id_, "");
    for (const auto& item : items)
    {
        QString page_id = item[0], page_name = item[1], sig_name = item[2], sig_width = item[3], page_count = item[4];
        if (item[5] == "1")
        {
            signal_naming.update_key("${BLOCK_NAME}", item[6]);
            signal_naming.update_key("${BLOCK_ABBR}", item[7]);
            sig_name = signal_naming.get_extended_name(sig_name);
        }
        QVector<QVector<QString> > regs;
        success_ = success_ && DataBaseHandler::show_items("chip_register_page_content", {"reg_id"}, "page_id", page_id, regs);
        for (const auto& reg : regs)
        {
            QString reg_id = reg[0];
            reg_id2page[reg_id] = {page_name, page_count, sig_name, sig_width};
        }
    }
    return reg_id2page;
}
