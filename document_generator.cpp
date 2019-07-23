#include "document_generator.h"
#include "global_variables.h"
#include "database_handler.h"
#include "data_utils.h"
#include "authenticator.h"
#include <QDebug>

QVector<QString> DocumentGenerator::doc_types_;
bool DocumentGenerator::success_;

DocumentGenerator::DocumentGenerator(const QString& chip_id_,
                  const QString& chip_name_,
                  int address_width_,
                  int register_width_,
                  bool msb_first_,
                  const QString& user_id_,
                  Authenticator* authenticator_,
                  const CAPTION_POSITION& img_pos,
                  const CAPTION_POSITION& tab_pos,
                  const QString& show_paged_register):
    chip_name_(chip_name_),
    chip_id_(chip_id_),
    user_id_(user_id_),
    register_width_(register_width_),
    address_width_(address_width_),
    msb_first_(msb_first_),
    authenticator_(authenticator_),
    image_cap_pos_(img_pos),
    table_cap_pos_(tab_pos),
    show_paged_register_(show_paged_register)
{
    reset_success_flag();
}

void DocumentGenerator::add_doc_type(const QString& doc_type)
{
    doc_types_.append(doc_type);
}

void DocumentGenerator::reset_success_flag()
{
    success_ = true;
}

bool DocumentGenerator::success()
{
    return success_;
}

QString DocumentGenerator::generate_html(const QString& doc_type, const QString& content, const CAPTION_POSITION& img_pos, const CAPTION_POSITION& tab_pos)
{
    return generate_doc(doc_type,
                        content,
                        &DocumentGenerator::generate_text_html,
                        &DocumentGenerator::generate_image_html,
                        &DocumentGenerator::generate_table_html,
                        img_pos, tab_pos);
}

QString DocumentGenerator::generate_text_html(const QString& text)
{
    QString text_html = HTML_TEXT_TEMPLATE;
    text_html.replace("${CONTENT}", text);
    return text_html;
}

QString DocumentGenerator::generate_image_html(const QString& caption, const QString& width, const QString& path, const CAPTION_POSITION& pos)
{
    QString image = HTML_TABLE_TEMPLATE;
    image.replace("${IMAGE}", path).replace("${WIDTH}", QString::number(static_cast<int>(width.toDouble() * 100)) + "%").replace("${CAPTION}", caption);
    if (pos == CAPTION_POSITION::TOP) image.replace("${CAPTION_TOP}", QString("<figcaption>${CAPTION}</figcaption>\n").replace("${CAPTION}", caption)).replace("${CAPTION_BOTTOM}", "");
    else image.replace("${CAPTION_TOP}", "").replace("${CAPTION_BOTTOM}", QString("<figcaption>${CAPTION}</figcaption>\n").replace("${CAPTION}", caption));
    return image;
}

QString DocumentGenerator::generate_table_html(const QString& caption, const QVector<QVector<QString> >& cells, const CAPTION_POSITION& pos)
{
    QString table = HTML_IMAGE_TEMPLATE;
    QString table_content;
    int rows = cells.size();
    int cols = rows > 0 ? cells[0].size() : 0;
    if (rows < 1 or cols < 1) return "";

    if (caption != "")
        table_content += ("<caption id=tab>" + caption + "</caption>");

    table_content += "<tr>";
    for (int j = 0; j < cols; j++)
        table_content += ("<th>" + cells[0][j] + "</th>");
    table_content += "</tr>";
    for (int i = 1; i < rows; i++)
    {
        table_content += "<tr>";
        for (int j = 0; j < cols; j ++)
            table_content += ("<td>" + cells[i][j] + "</td>");
        table_content += "</tr>";
    }

    table.replace("${TABLE}", table_content);
    if (pos == CAPTION_POSITION::TOP) table.replace("${CAPTION_POS}", "top");
    else if (pos == CAPTION_POSITION::BOTTOM) table.replace("${CAPTION_POS}", "bottom");
    return table;
}

QString DocumentGenerator::generate_tex(const QString &doc_type, const QString &content, const CAPTION_POSITION& img_pos, const CAPTION_POSITION& tab_pos)
{
    return generate_doc(doc_type,
                        content,
                        &DocumentGenerator::generate_text_tex,
                        &DocumentGenerator::generate_image_tex,
                        &DocumentGenerator::generate_table_tex,
                        img_pos, tab_pos);
}

QString DocumentGenerator::generate_text_tex(const QString &text)
{
    int mathMode = 0;
    QString t;
    int last = 0;

    for (int i = 0; i <= text.size(); i++)
    {
        if (i + 1 <= text.size() && get_previous_k_chars(i+1, 2, text) == "$$")
        {
            if (mathMode) t = t + text.mid(last, i - last - 1) + "$$";
            else t = t + pure_text_to_tex(text.mid(last, i - last -1 ) )+ "$$";
            mathMode = mathMode ? 0 : 1;
            i++;
            last = i;
            continue;
        }

        QString prev1 = get_previous_k_chars(i, 1, text),
                prev2 = get_previous_k_chars(i, 2, text),
                prev4 = get_previous_k_chars(i, 4, text),
                prev6 = get_previous_k_chars(i, 6, text);

        if (!mathMode)
        {
            if (prev1 == "$" && prev2 != "\\$") t = t + pure_text_to_tex(text.mid(last, i - last - 1)) + prev1;
            else if (prev2 == "\\(" || prev2 == "\\[") t = t + pure_text_to_tex(text.mid(last, i - last - 2)) + prev2;
            else if (prev6 == "\\begin") t = t + pure_text_to_tex(text.mid(last, i - last - 6)) + prev6;
            if ((prev1 == "$" && prev2 != "\\$") || prev2 == "\\(" || prev2 == "\\[" || prev6 == "\\begin")
            {
                mathMode = 1;
                last = i;
            }
        }
        else {
            if (prev6 == "\\begin") mathMode++;
            else {
                if ((prev1 == "$" && prev2 != "\\$") || prev2 == "$$" ||
                        prev2 == "\\)" || prev2 == "\\]" || prev4 == "\\end")
                    mathMode--;

                if (prev1 == "$" && prev2 != "\\$" && !mathMode) t = t + text.mid(last, i - last - 1) + prev1;
                else if (( prev2 == "\\)" || prev2 == "\\]") && !mathMode) t = t + text.mid(last, i - last - 2) + prev2;
                else if (prev4 == "\\end" && !mathMode)
                {
                    int j = i;
                    while (j < text.size() && text[j] == " ") j++;
                    if (j < text.size() && text[j] == "{")
                    {
                        while ( j < text.size() && text[j] != "}") j++;
                        if (j < text.size() && text[j] == "}")
                        {
                            t = t + text.mid(last, j+1 - last);
                            i = j+1;
                            last = j+1;
                            continue;
                        }
                    }

                    t = t + text.mid(last, i - last - 4) + prev4;
                }
                if (!mathMode) last = i;
            }
        }
    }

    t = t + (mathMode ? text.mid(last, text.size() - last) : pure_text_to_tex(text.mid(last, text.size() - last)));
    return t;
}

QString DocumentGenerator::generate_table_tex(const QString &caption, const QVector<QVector<QString> > &cells, const CAPTION_POSITION& pos)
{
    int rows = cells.size();
    int cols = rows > 0 ? cells[0].size() : 0;
    if (rows < 1 or cols < 1) return "";

    QString title;
    if (caption != "")
    {
        title = "\\caption {${CAPTION}} \\label{tab:${RAW_CAPTION}}\n";
        title.replace("${CAPTION}", generate_text_tex(caption)).replace("${RAW_CAPTION}", caption);
    }
    QString header = generate_text_tex(cells[0][0]);
    for (int j = 1; j < cols; j++)
        header += (" & " + generate_text_tex(cells[0][j]));
    header += " \\\\\n";
    header = "\\hline\n" + header + "\\hline\n";

    QString content;
    for (int i = 1; i < rows; i++)
    {
        content += generate_text_tex(cells[i][0]);
        for (int j = 1; j < cols; j++)
            content += (" & " + generate_text_tex(cells[i][j]));
        content += " \\\\\n";
    }
    content += "\\hline\n";
    content = header + content;

    QString table = "\\begin{tabular}{" + QString(cols, 'l') + "}\n" + content + "\\end{tabular}\n";
    table = "\\begin{center}\n" + table + "\\end{center}\n";
    if (caption != "" && pos == CAPTION_POSITION::TOP)
        table = title + table;
    else if (caption != "" && pos == CAPTION_POSITION::BOTTOM)
        table = table + title;
    table = "\\begin{table}[htbp]\n" + table + "\\end{table}";
    return table;
}

QString DocumentGenerator::generate_image_tex(const QString &caption, const QString &width, const QString &path, const CAPTION_POSITION& pos)
{
    QString image = "\\includegraphics[width = ${WIDTH}\\textwidth]{${PATH}}\n";
    if (caption != "" && pos == CAPTION_POSITION::TOP)
        image = "\\caption{${CAPTION}\\label{fig:${RAW_CAPTION}}}\n" + image;
    if (caption != "" && pos == CAPTION_POSITION::BOTTOM)
        image += "\\caption{${CAPTION}\\label{fig:${RAW_CAPTION}}}\n";
    image.replace("${PATH}", path).replace("${CAPTION}", generate_text_tex(caption)).replace("${RAW_CAPTION}", caption).replace("${WIDTH}", width);
    image = "\\begin{center}\n" + image + "\\end{center}\n";
    image = "\\begin{figure}[htbp]\n" + image + "\\end{figure}\n";
    return image;
}

QString DocumentGenerator::generate_html_document()
{
    reset_success_flag();
    QString chip_content = generate_chip_level_html_document();
    QString html_content, table_of_content;
    QVector<QVector<QString> > items;
    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator_->can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id_}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id_}, {"block_system_block.responsible", user_id_}};
    success_ = success_ && DataBaseHandler::show_items_inner_join({"block_system_block.block_id",
                                     "block_system_block.block_name",
                                     "block_system_block.abbreviation",
                                    "block_system_block.start_address", "block_system_block.prev", "block_system_block.next"},
                                    {{{"block_system_block", "responsible"}, {"global_user", "user_id"}}},
                                    items, key_value_pairs,
                                    "order by block_system_block.block_id");
    if (authenticator_->can_read_all_blocks()) items = sort_doubly_linked_list(items);

    QHash<QString, QVector<QString> > reg_id2page = get_register_id2page();

    for (const auto& item : items)
    {
        QString block_content = generate_block_level_html_document(item[0],
                                                           item[1],
                                                           item[2],
                                                           item[3], reg_id2page);
        QString reg_bullets = block_content.split(DOC_DELIMITER)[0];
        block_content = block_content.right(block_content.size() - reg_bullets.size() - DOC_DELIMITER.size());
        html_content += block_content;
        table_of_content += QString("<li><a href=#${BLOCK_NAME}>${BLOCK_NAME}</a></li>\n").replace("${BLOCK_NAME}", item[1]) + reg_bullets;
    }
    table_of_content = "<ol>\n" + table_of_content + "</ol>\n";
    html_content = chip_content + table_of_content + html_content;
    QString html = HTML_TEMPLATE;
    html.replace("${HTML}", html_content).replace("${MATHJAX_ROOT}", MATHJAX_ROOT);
    return html;
}


QString DocumentGenerator::generate_tex_document()
{
    reset_success_flag();
    QString chip_content = generate_chip_level_tex_document();
    QString tex_content;
    QVector<QVector<QString> > items;
    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator_->can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id_}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id_}, {"block_system_block.responsible", user_id_}};
    success_ = success_ && DataBaseHandler::show_items_inner_join({"block_system_block.block_id",
                                     "block_system_block.block_name",
                                     "block_system_block.abbreviation",
                                    "block_system_block.start_address", "block_system_block.prev", "block_system_block.next"},
                                    {{{"block_system_block", "responsible"}, {"global_user", "user_id"}}},
                                    items, key_value_pairs,
                                    "order by block_system_block.block_id");
    if (authenticator_->can_read_all_blocks()) items = sort_doubly_linked_list(items);

    QHash<QString, QVector<QString> > reg_id2page = get_register_id2page();

    for (const auto& item : items)
    {
        QString block_content = generate_block_level_tex_document(item[0],
                                                           item[1],
                                                           item[2],
                                                           item[3], reg_id2page);
        tex_content += block_content;
    }
    return chip_content + tex_content;
}

QString DocumentGenerator::generate_chip_level_html_document()
{
    QVector<QVector<QString> > items;

    QString chip_title = "<h1 id=" + chip_name_ +">"+ chip_name_ +"</h1>\n";
    QString chip_content;
    success_ = success_ && DataBaseHandler::show_items_inner_join({"doc_chip.chip_doc_id", "def_doc_type.doc_type", "doc_chip.content", "doc_chip.doc_type_id", "doc_chip.prev", "doc_chip.next"},
                                    {{{"doc_chip", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_chip.chip_id", chip_id_}});
    for (const auto& item : items)
        chip_content = chip_content + generate_html(item[1], item[2], image_cap_pos_, table_cap_pos_) + '\n';
    return chip_title + chip_content;
}

QString DocumentGenerator::generate_chip_level_tex_document()
{
    QVector<QVector<QString> > items;
    QString tex_content;

    QString chip_title = QString("\\chapter{${CHIP_NAME}}\n").replace("${CHIP_NAME}", pure_text_to_tex(chip_name_));
    QString chip_content;
    success_ = success_ && DataBaseHandler::show_items_inner_join({"doc_chip.chip_doc_id", "def_doc_type.doc_type", "doc_chip.content", "doc_chip.doc_type_id", "doc_chip.prev", "doc_chip.next"},
                                    {{{"doc_chip", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_chip.chip_id", chip_id_}});

    for (int i = 0; i < items.size(); i++)
    {
        const auto& item = items[i];
        chip_content = chip_content + generate_tex(item[1], item[2], image_cap_pos_, table_cap_pos_);
        if (i != items.size() - 1) chip_content += "~\\\\";
        chip_content += "\n";
    }
    return chip_title + chip_content;
}


QString DocumentGenerator::generate_block_level_html_document(const QString &block_id,
                                                              const QString &block_name,
                                                              const QString &block_abbr,
                                                              const QString &block_start_addr,
                                                              const QHash<QString, QVector<QString> >& reg_id2page)
{
    quint64 block_start_address_decimal = block_start_addr.toULongLong(nullptr, 16);
    NamingTemplate register_naming = GLOBAL_REGISTER_NAMING,
            signal_naming = GLOBAL_SIGNAL_NAMING;
    register_naming.update_key("${BLOCK_NAME}", block_name);
    register_naming.update_key("${BLOCK_ABBR}", block_abbr);
    signal_naming.update_key("${BLOCK_NAME}", block_name);
    signal_naming.update_key("${BLOCK_ABBR}", block_abbr);

    QVector<QVector<QString> > items;
    success_ = success_ && DataBaseHandler::show_items_inner_join({"doc_block.block_doc_id", "def_doc_type.doc_type", "doc_block.content", "doc_block.doc_type_id", "doc_block.prev", "doc_block.next"},
                                    {{{"doc_block", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_block.block_id", block_id}});

    items = sort_doubly_linked_list(items);
    QString title = "<h2 id=" + block_name +">"+ block_name +"</h2>\n";
    QString block_content;
    for (const auto& item : items)
        block_content = block_content + generate_html(item[1], item[2], image_cap_pos_, table_cap_pos_) + '\n';

    QVector<QVector<QString> > registers;
    success_ = success_ && DataBaseHandler::show_items("block_register", {"reg_id", "reg_name", "prev", "next"}, "block_id", block_id, registers);
    registers = sort_doubly_linked_list(registers);

    QString reg_bullets;
    for (int i = 0; i < registers.size(); i++)
    {
        const auto& reg = registers[i];
        QString reg_id = reg[0], reg_name = register_naming.get_extended_name(reg[1]);
        QString address = decimal2hex(block_start_address_decimal + static_cast<quint64>(i), address_width_);
        block_content += generate_register_level_html_document(reg_id, reg_name, address, signal_naming, reg_id2page);
        reg_bullets += QString("<li><a href=#${REG_NAME}>${REG_NAME}</a>\n</li>\n").replace("${REG_NAME}", reg_name);
    }
    reg_bullets = "<ul>\n" + reg_bullets + "</ul>\n";
    return reg_bullets + DOC_DELIMITER + title + "<section>\n" + block_content + "</section>\n";
}

QString DocumentGenerator::generate_block_level_tex_document(const QString &block_id,
                                                             const QString &block_name,
                                                             const QString &block_abbr,
                                                             const QString &block_start_addr,
                                                             const QHash<QString, QVector<QString> >& reg_id2page)
{
    quint64 block_start_address_decimal = block_start_addr.toULongLong(nullptr, 16);

    NamingTemplate register_naming = GLOBAL_REGISTER_NAMING,
            signal_naming = GLOBAL_SIGNAL_NAMING;
    register_naming.update_key("${BLOCK_NAME}", block_name);
    register_naming.update_key("${BLOCK_ABBR}", block_abbr);
    signal_naming.update_key("${BLOCK_NAME}", block_name);
    signal_naming.update_key("${BLOCK_ABBR}", block_abbr);

    QVector<QVector<QString> > items;
    success_ = success_ && DataBaseHandler::show_items_inner_join({"doc_block.block_doc_id", "def_doc_type.doc_type", "doc_block.content", "doc_block.doc_type_id", "doc_block.prev", "doc_block.next"},
                                    {{{"doc_block", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_block.block_id", block_id}});

    items = sort_doubly_linked_list(items);
    QString title = QString("\\section{${BLOCK_NAME}}\n").replace("${BLOCK_NAME}", pure_text_to_tex(block_name));
    QString block_content;
    for (int i = 0; i < items.size(); i++)
    {
        const auto& item = items[i];
        block_content = block_content + generate_tex(item[1], item[2], image_cap_pos_, table_cap_pos_);
        if (i != items.size() - 1) block_content += "~\\\\";
        block_content += "\n";
    }

    QVector<QVector<QString> > registers;
    success_ = success_ && DataBaseHandler::show_items("block_register", {"reg_id", "reg_name", "prev", "next"}, "block_id", block_id, registers);
    registers = sort_doubly_linked_list(registers);

    for (int i = 0; i < registers.size(); i++)
    {
        const auto& reg = registers[i];
        QString reg_id = reg[0], reg_name = register_naming.get_extended_name(reg[1]);
        QString address = decimal2hex(block_start_address_decimal + static_cast<quint64>(i), address_width_);
        block_content += generate_register_level_tex_document(reg_id, reg_name, address, signal_naming, reg_id2page);
    }
    return title + block_content;
}

QString DocumentGenerator::generate_register_level_html_document(const QString &reg_id,
                                                                 const QString &reg_name,
                                                                 const QString& address,
                                                                 const NamingTemplate& signal_naming,
                                                                 const QHash<QString, QVector<QString> >& reg_id2page)
{

    QString register_content = "<h4 id=" + reg_name + ">" + reg_name +" - " + address;
    if (reg_id2page.contains(reg_id))
    {
        if (show_paged_register_ == "page_name") register_content += " - " + reg_id2page[reg_id][0];
        else register_content += " - " + reg_id2page[reg_id][2];
    }
    register_content += "</h4>\n";
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

    if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() > b[2].toInt();});
    else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toInt() < b[1].toInt();});

    register_content = register_content + generate_register_bit_table_html(items, signal_naming);

    QSet<QString> signal_set;
    QVector<QString> signal_ids, signal_names;
    for (const auto& signal_item : items)
    {
        QString sig_id = signal_item[4],
                sig_name = signal_item[9]=="1" ? signal_naming.get_extended_name(signal_item[3]) : signal_item[3];
        if (signal_set.contains(sig_id)) continue;
        signal_set.insert(sig_id);
        signal_ids.push_back(sig_id);
        signal_names.push_back(sig_name);
    }

    items.clear();
    success_ = success_ && DataBaseHandler::show_items_inner_join({"doc_register.register_doc_id", "def_doc_type.doc_type", "doc_register.content", "doc_register.doc_type_id", "doc_register.prev", "doc_register.next"},
                                    {{{"doc_register", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_register.reg_id", reg_id}});
    items = sort_doubly_linked_list(items);
    for (const auto& item : items)
    {
        register_content = register_content + generate_html(item[1], item[2], image_cap_pos_, table_cap_pos_) + '\n';
    }
    register_content += generate_register_signal_bullets_html(signal_ids, signal_names);
    for (const QString& sig_id : signal_ids) seen_signal2reg_[sig_id] = reg_name;
    return register_content;
}

QString DocumentGenerator::generate_register_level_tex_document(const QString &reg_id,
                                                                const QString &reg_name,
                                                                const QString& address,
                                                                const NamingTemplate& signal_naming,
                                                                const QHash<QString, QVector<QString> >& reg_id2page)
{
    QString register_content = QString("${REG_NAME} - ${ADDR}").replace("${REG_NAME}", pure_text_to_tex(reg_name)).replace("${ADDR}", address);
    if (reg_id2page.contains(reg_id))
    {
        if (show_paged_register_ == "page_name") register_content += " - " + pure_text_to_tex(reg_id2page[reg_id][0]);
        else register_content += " - " + pure_text_to_tex(reg_id2page[reg_id][2]);
    }
    register_content = QString("\\paragraph*{${CONTENT}}~\\\\~\\\\ \n").replace("${CONTENT}", register_content);

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

    if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() > b[2].toInt();});
    else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toInt() < b[1].toInt();});

    register_content += generate_register_bit_table_tex(items, signal_naming);

    QSet<QString> signal_set;
    QVector<QString> signal_ids, signal_names;
    for (const auto& signal_item : items)
    {
        QString sig_id = signal_item[4],
                sig_name = signal_item[9] == "1" ? signal_naming.get_extended_name(signal_item[3]) : signal_item[3];
        if (signal_set.contains(sig_id)) continue;
        signal_set.insert(sig_id);
        signal_ids.push_back(sig_id);
        signal_names.push_back(sig_name);
    }

    items.clear();
    success_ = success_ && DataBaseHandler::show_items_inner_join({"doc_register.register_doc_id", "def_doc_type.doc_type", "doc_register.content", "doc_register.doc_type_id", "doc_register.prev", "doc_register.next"},
                                    {{{"doc_register", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_register.reg_id", reg_id}});
    items = sort_doubly_linked_list(items);
    if (items.size() > 0) register_content += "~\\\\~\\\\\n";
    for (int i = 0; i < items.size(); i++)
    {
        const auto& item = items[i];
        register_content += generate_tex(item[1], item[2], image_cap_pos_, table_cap_pos_);
        if (i != items.size() - 1) register_content += "~\\\\";
        register_content += "\n";
    }
    register_content += generate_register_signal_bullets_tex(signal_ids, signal_names);
    for (const QString& sig_id : signal_ids) seen_signal2reg_[sig_id] = reg_name;
    return register_content;
}

QString DocumentGenerator::generate_register_bit_table_html(const QVector<QVector<QString> >& signal_items, const NamingTemplate& signal_naming)
{
    QString reg_bit_table = "<style>\n\
            table#t_reg, td#t_reg {border: 1px solid black;border-collapse: collapse;}\n\
            td#t_reg {padding: 2px;text-align: center;}\n\
            </style>\n";

    QString header, signal_row, value_row;
    for (int j = 0; j < register_width_; j++)
    {
        if (msb_first_) header += "<td id=t_reg width=" + QString::number(100/register_width_) + "%>" + QString::number(register_width_-1-j) + "</td>\n";
        else header += "<td id=t_reg width=" + QString::number(100/register_width_) + "%>" + QString::number(j) + "</td>\n";
    }
    header = "<tr id=t_reg>\n" + header + "</tr>\n";

    if (signal_items.size() == 0) {
        QString span = QString::number(register_width_);
        QString cell = "<td id=t_reg colspan=" +span + ">" + "..." + "</td>\n";
        signal_row = cell;
        for (int j = 0; j < register_width_; j++) value_row += "<td id=t_reg>.</td>\n";
    }
    else {
        for (int j = 0; j < signal_items.size(); j++)
        {
            const auto &curr = signal_items[j];
            QString reg_msb = curr[2],
                    reg_lsb = curr[1],
                    sig_msb = curr[6],
                    sig_lsb = curr[5],
                    sig_name = curr[9]=="1" ? signal_naming.get_extended_name(curr[3]) : curr[3],
                    sig_value = QString::number(curr[7].toULongLong(nullptr, 16), 2);
            int sig_width = curr[8].toInt();
            sig_value = QString(sig_width - sig_value.size(), '0') + sig_value;
            QString span = QString::number(reg_msb.toInt() - reg_lsb.toInt() + 1);
            signal_row += "<td id=t_reg colspan=" +span + ">" + "<font size=\"2\">" + sig_name +"</font>" + "</td>";

            if (msb_first_)
                for (int bit = sig_msb.toInt(); bit >= sig_lsb.toInt(); bit--) value_row += "<td id=t_reg>" + sig_value[sig_width-bit-1];
            else
                for (int bit = sig_lsb.toInt(); bit <= sig_msb.toInt(); bit++) value_row += "<td id=t_reg>" + sig_value[sig_width-bit-1];
            if (j < signal_items.size()-1)
            {
                const auto &next = signal_items[j+1];
                int gap = msb_first_ ? reg_lsb.toInt() - next[2].toInt() - 1 : next[1].toInt() - reg_msb.toInt() -1;
                if (gap > 0)
                {
                    signal_row += "<td id=t_reg colspan=" + QString::number(gap) + ">" + "..." + "</td>";
                    for (int bit = 0; bit < gap; bit ++) value_row += "<td id=t_reg>.</td>\n";
                }

            }
            if (j == 0)
            {
                int gap = msb_first_ ? register_width_ - reg_msb.toInt() -1 : reg_lsb.toInt();
                if (gap > 0)
                {
                    signal_row = "<td id=t_reg colspan=" +QString::number(gap) + ">" + "..." + "</td>" + signal_row;
                    for (int bit = 0; bit < gap; bit ++) value_row = "<td id=t_reg>.</td>\n" + value_row;
                }
            }
            if (j == signal_items.size()-1)
            {
                int gap = msb_first_ ? reg_lsb.toInt() : register_width_ - reg_msb.toInt() -1;
                if (gap > 0)
                {
                    signal_row += "<td id=t_reg colspan=" +QString::number(gap) + ">" + "..." + "</td>";
                    for (int bit = 0; bit < gap; bit ++) value_row += "<td id=t_reg>.</td>\n";
                }
            }
        }
    }
    signal_row = "<tr id=t_reg>\n" + signal_row + "</tr>\n";
    value_row = "<tr id=t_reg>\n" + value_row + "</tr>\n";
    reg_bit_table = "<p>\n<table id=t_reg width=90%>\n" + reg_bit_table + header + signal_row + value_row + "</table>\n</p>\n";
    return reg_bit_table;
}

QString DocumentGenerator::generate_register_bit_table_tex(const QVector<QVector<QString> > &signal_items, const NamingTemplate& signal_naming)
{
    QString reg_bit_table = QString("\\begin{sffamily} \\begin{tiny} \n\\begin{bytefield}[bitwidth=${BIT_WIDTH}\\linewidth]{${BITS}} \n");
    reg_bit_table.replace("${BIT_WIDTH}", QString::number(1.0/register_width_)).replace("${BITS}", QString::number(register_width_));

    QString header, signal_row, value_row;
    for (int j = 0; j < register_width_; j++)
    {
        if (j == 0) header += msb_first_ ? QString::number(register_width_-j-1) : QString::number(j);
        else header = header + "," + (msb_first_ ? QString::number(register_width_-j-1) : QString::number(j));
    }
    header = "\\bitheader{" + header + "} \\\\\n";

    if (signal_items.size() == 0) {
        signal_row = QString("\\bitbox{${BITS}}{{****}} \\\\\n").replace("${BITS}", QString::number(register_width_));
        for (int j = 0; j < register_width_; j++)
            if (j != register_width_ -1 ) value_row += QString("\\bitbox{1}{{.}} &\n");
            else value_row += QString("\\bitbox{1}{{.}} \n");
    }
    else {
        for (int j = 0; j < signal_items.size(); j++)
        {
            const auto &curr = signal_items[j];
            QString reg_msb = curr[2],
                    reg_lsb = curr[1],
                    sig_msb = curr[6],
                    sig_lsb = curr[5],
                    sig_name = curr[9]=="1" ? signal_naming.get_extended_name(curr[3]) : curr[3],
                    sig_value = QString::number(curr[7].toULongLong(nullptr, 16), 2);
            int sig_width = curr[8].toInt();
            sig_value = QString(sig_width - sig_value.size(), '0') + sig_value;
            QString span = QString::number(reg_msb.toInt() - reg_lsb.toInt() + 1);
            signal_row += QString("\\bitbox{${SPAN}}{{${SIG_NAME}}}").replace("${SPAN}", span).replace("${SIG_NAME}", pure_text_to_tex(sig_name));

            if ((msb_first_ && reg_lsb.toInt() == 0) || (!msb_first_ && reg_msb.toInt() == register_width_ -1)) // signal reaches last bit of reg
                signal_row += " \\\\\n";
            else signal_row += " &\n";

            if (msb_first_)
            {
                for (int bit = sig_msb.toInt(); bit >= sig_lsb.toInt(); bit--)
                {
                    value_row += QString("\\bitbox{1}{{${VAL}}}").replace("${VAL}", QString(sig_value[sig_width-bit-1]));
                    if (bit == sig_lsb.toInt() && sig_lsb.toInt() == 0)   // last bit of reg
                        value_row += " \n";
                    else value_row += " &\n";
                }
            }
            else
            {
                for (int bit = sig_lsb.toInt(); bit <= sig_msb.toInt(); bit++)
                {
                    value_row += QString("\\bitbox{1}{{${VAL}}}").replace("${VAL}", QString(sig_value[sig_width-bit-1]));
                    if (bit == sig_msb.toInt() && sig_msb.toInt() == register_width_ - 1)   // last bit of reg
                        value_row += " \n";
                    else value_row += " &\n";
                }
            }
            if (j < signal_items.size()-1)
            {
                const auto &next = signal_items[j+1];
                int gap = msb_first_ ? reg_lsb.toInt() - next[2].toInt() - 1 : next[1].toInt() - reg_msb.toInt() -1;
                if (gap > 0)
                {
                    signal_row += QString("\\bitbox{${SPAN}}{{${SIG_NAME}}} &\n").replace("${SPAN}", QString::number(gap)).replace("${SIG_NAME}", "****");
                    for (int bit = 0; bit < gap; bit ++) value_row += QString("\\bitbox{1}{{.}} &\n");
                }

            }
            if (j == 0)
            {
                int gap = msb_first_ ? register_width_ - reg_msb.toInt() -1 : reg_lsb.toInt();
                if (gap > 0)
                {
                    signal_row = QString("\\bitbox{${SPAN}}{{${SIG_NAME}}} &\n").replace("${SPAN}", QString::number(gap)).replace("${SIG_NAME}", "****") + signal_row;
                    for (int bit = 0; bit < gap; bit ++) value_row = QString("\\bitbox{1}{{.}} &\n") + value_row;
                }
            }
            if (j == signal_items.size()-1)
            {
                int gap = msb_first_ ? reg_lsb.toInt() : register_width_ - reg_msb.toInt() -1;
                if (gap > 0)
                {
                    signal_row += QString("\\bitbox{${SPAN}}{{${SIG_NAME}}} \\\\\n").replace("${SPAN}", QString::number(gap)).replace("${SIG_NAME}", "****");
                    for (int bit = 0; bit < gap; bit ++)
                    {
                        if (bit != gap -1 ) value_row += QString("\\bitbox{1}{{.}} &\n");
                        else value_row += QString("\\bitbox{1}{{.}} \n");
                    }
                }
            }
        }
    }
    reg_bit_table = reg_bit_table + header + signal_row + value_row + "\\end{bytefield} \n\\end{tiny} \\end{sffamily} \n";
    return reg_bit_table;
}

QString DocumentGenerator::generate_register_signal_bullets_html(const QVector<QString>& signal_ids, const QVector<QString>& signal_names)
{
    QVector<QVector<QString> > doc_items;
    QString bullet_list;
    for (int j = 0; j < signal_ids.size(); j++)
    {
        QString sig_id = signal_ids[j], sig_name = signal_names[j];
        if (seen_signal2reg_.contains(sig_id))
        {
            QString bullet = sig_name + " see " + seen_signal2reg_[sig_id];
            bullet = "<li>" + bullet + "</li>\n";
            bullet_list += bullet;
            continue;
        }
        success_ = success_ && DataBaseHandler::show_items_inner_join({"doc_signal.signal_doc_id", "def_doc_type.doc_type", "doc_signal.content", "doc_signal.doc_type_id", "doc_signal.prev", "doc_signal.next"},
                                        {{{"doc_signal", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        doc_items, {{"doc_signal.sig_id", sig_id}});
        if (doc_items.size() > 0)
        {
            doc_items = sort_doubly_linked_list(doc_items);
            QString bullet = sig_name + " ";
            for (const auto& doc_item : doc_items)
            {
                bullet += generate_html(doc_item[1], doc_item[2], image_cap_pos_, table_cap_pos_) +"\n" ;
            }
            bullet = "<li>" + bullet + "</li>\n";
            bullet_list += bullet;
        }
    }
    bullet_list = "<ul>\n" + bullet_list + "</ul>\n";
    return bullet_list;
}

QString DocumentGenerator::generate_register_signal_bullets_tex(const QVector<QString>& signal_ids, const QVector<QString>& signal_names)
{
    QString bullet_list;
    QVector<QVector<QString> > doc_items;
    for (int j = 0; j < signal_ids.size(); j++)
    {
        QString sig_id = signal_ids[j], sig_name = signal_names[j];
        if (seen_signal2reg_.contains(sig_id))
        {
            QString bullet = pure_text_to_tex(sig_name) + " see " + pure_text_to_tex(seen_signal2reg_[sig_id]);
            bullet = "\\item " + bullet;
            bullet_list += bullet;
            continue;
        }

        success_ = success_ && DataBaseHandler::show_items_inner_join({"doc_signal.signal_doc_id", "def_doc_type.doc_type", "doc_signal.content", "doc_signal.doc_type_id", "doc_signal.prev", "doc_signal.next"},
                                        {{{"doc_signal", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        doc_items, {{"doc_signal.sig_id", sig_id}});
        if (doc_items.size() > 0)
        {
            doc_items = sort_doubly_linked_list(doc_items);
            QString bullet = pure_text_to_tex(sig_name) + " ";
            for (int k = 0; k < doc_items.size(); k++)
            {
                const auto& doc_item = doc_items[k];
                QString item = generate_tex(doc_item[1], doc_item[2], image_cap_pos_, table_cap_pos_);
                if (k != doc_items.size() -1) item += " ~\\\\";
                bullet += item +"\n" ;
            }
            bullet = "\\item " + bullet;
            bullet_list += bullet;
        }
    }
    if (bullet_list != "") bullet_list = "\\begin{itemize}\\itemsep-5pt\n" + bullet_list + "\\end{itemize}\n\n\n";
    return bullet_list;
}

QString DocumentGenerator::get_previous_k_chars(int i, int k, const QString &text)
{
    if (i >= k) return text.mid(i - k, k);
    else return text.mid(0, i);
}

QString DocumentGenerator::pure_text_to_tex(const QString &text)
{
    QString tex = text;
    //tex.replace('\\', "\textbackslash ");
    tex.replace('^', "\textasciicircum ");
    tex.replace('~', "\textasciitilde ");
    for (QChar c : {'#', '$', '%', '&', '_', '{', '}'})
        tex.replace(c, '\\' + c);
    return tex;
}

QString DocumentGenerator::generate_doc(const QString &doc_type, const QString &content,
                                         QString (*f_text)(const QString &),
                                         QString (*f_img)(const QString &, const QString &, const QString &, const CAPTION_POSITION& ),
                                         QString (*f_tab)(const QString &, const QVector<QVector<QString> > &, const CAPTION_POSITION&),
                                         const CAPTION_POSITION& img_pos, const CAPTION_POSITION& tab_pos)
{
    if (doc_types_.size() != 3) return "";
    if (doc_type == doc_types_[0]) // text
    {
        QString text = content;
        return f_text(text);
    }
    else if (doc_type == doc_types_[1]) // image
    {
        QStringList ss = content.split(DOC_DELIMITER);
        QString caption = ss[0], width = ss[1], path = ss[2];
        return f_img(caption, width, path, img_pos);
    }
    else if (doc_type == doc_types_[2]) // table
    {
        QStringList ss = content.split(DOC_DELIMITER);
        QString caption = ss[0];
        int rows = ss[1].toInt(), cols = ss[2].toInt();
        QVector<QVector<QString> > cells(rows, QVector<QString>(cols, ""));
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                cells[i][j] = ss[i*cols + j + 3];
        return f_tab(caption, cells, tab_pos);
    }
    return "";
}

QHash<QString, QVector<QString> > DocumentGenerator::get_register_id2page() const
{
    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING;
    QVector<QVector<QString> > items;
    QHash<QString, QVector<QString> > reg_id2page;
    DataBaseHandler::show_items_inner_join({"chip_register_page.page_id",
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
        DataBaseHandler::show_items("chip_register_page_content", {"reg_id"}, "page_id", page_id, regs);
        for (const auto& reg : regs)
        {
            QString reg_id = reg[0];
            reg_id2page[reg_id] = {page_name, page_count, sig_name, sig_width};
        }
    }
    return reg_id2page;
}
