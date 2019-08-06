#include "database_utils.h"
#include "global_variables.h"
#include "data_utils.h"
#include "database_handler.h"
#include <QSet>

DatabaseUtils::DatabaseUtils()
{

}


bool DatabaseUtils::remove_signal(const QString& sig_id, const QString& reg_sig_id)
{
    QVector<QVector<QString> > page_ids;
    if (!DataBaseHandler::show_items("chip_register_page", {"page_id"}, "ctrl_sig", sig_id, page_ids))
        return false;
    bool success = true;
    for (const auto& page_id : page_ids) success = success && DataBaseHandler::delete_items("chip_register_page_content", "page_id", page_id[0]);
    success = success && DataBaseHandler::delete_items("chip_register_page", "ctrl_sig", sig_id);
    success = success && DataBaseHandler::delete_items("doc_signal", "sig_id", sig_id);

    if (reg_sig_id != "")
    {
        success = success && DataBaseHandler::delete_items("block_sig_reg_partition_mapping", "reg_sig_id", reg_sig_id);
        success = success && DataBaseHandler::delete_items("signal_reg_signal", "reg_sig_id", reg_sig_id);
    }
    success = success && DataBaseHandler::delete_items("signal_signal", "sig_id", sig_id);

    return success;
}

bool DatabaseUtils::remove_register(const QString &reg_id)
{
    QVector<QString> item;

    if (!DataBaseHandler::show_one_item("block_register", item, {"prev", "next"}, "reg_id", reg_id) || item.size() == 0)
        return false;
    QString prev = item[0], next = item[1];
    bool success = true;

    success = success && DataBaseHandler::delete_items("block_sig_reg_partition_mapping", "reg_id", reg_id);
    success = success && DataBaseHandler::delete_items("doc_register", "reg_id", reg_id);
    success = success && DataBaseHandler::delete_items("chip_register_page_content", "reg_id", reg_id);
    success = success &&  DataBaseHandler::delete_items("block_register", "reg_id", reg_id);
    if (prev != "-1") success = success && DataBaseHandler::update_items("block_register", {{"reg_id", prev}}, {{"next", next}});
    if (next != "-1") success = success && DataBaseHandler::update_items("block_register", {{"reg_id", next}}, {{"prev", prev}});

    return success;
}

bool DatabaseUtils::remove_block(const QString &block_id)
{
    QVector<QString> item;
    bool success = true;
    if (!DataBaseHandler::show_one_item("block_system_block", item, {"prev", "next"}, "block_id", block_id) || item.size() == 0)
        return false;
    QString prev = item[0], next = item[1];

    success = success && DataBaseHandler::delete_items("doc_block", "block_id", block_id);

    QVector<QVector<QString> > items;
    success = success && DataBaseHandler::show_items("signal_signal", {"sig_id"}, "block_id", block_id, items);
    for(const auto& item: items)
    {
        QString sig_id = item[0];
        QVector<QString> reg_sig;
        success = success && DataBaseHandler::show_one_item("signal_reg_signal", reg_sig, {"reg_sig_id"}, "sig_id", sig_id);
        success = success && DatabaseUtils::remove_signal(sig_id, reg_sig.size() > 0 ? reg_sig[0] : "");
    }

    items.clear();
    success = success && DataBaseHandler::show_items("block_register", {"reg_id"}, "block_id", block_id, items);
    for(const auto& item: items) success = success && DatabaseUtils::remove_register(item[0]);

    success = success && DataBaseHandler::delete_items("block_system_block", "block_id", block_id);
    if (prev != "-1") success = success && DataBaseHandler::update_items("block_system_block", {{"block_id", prev}}, {{"next", next}});
    if (next != "-1") success = success && DataBaseHandler::update_items("block_system_block", {{"block_id", next}}, {{"prev", prev}});

    return success;
}

bool DatabaseUtils::remove_chip(const QString &chip_id)
{
    QVector<QVector<QString>> items;
    bool success = true;

    success = success && DataBaseHandler::delete_items("chip_designer", "chip_id", chip_id);
    success = success && DataBaseHandler::delete_items("doc_chip", "chip_id", chip_id);

    success = success && DataBaseHandler::show_items("chip_register_page", {"page_id"}, "chip_id", chip_id, items);
    for (const auto& item : items) success = success && DataBaseHandler::delete_items("chip_register_page_content", "page_id", item[0]);
    success = success && DataBaseHandler::delete_items("chip_register_page", "chip_id", chip_id);

    items.clear();
    success = success && DataBaseHandler::show_items("block_system_block", {"block_id"}, "chip_id", chip_id, items);
    for (const auto& item : items) success = success && DatabaseUtils::remove_block(item[0]);

    success = success && DataBaseHandler::delete_items("chip_chip", "chip_id", chip_id);

    return success;
}

bool DatabaseUtils::remove_user(const QString& my_user_id, const QString &user_id)
{
    QVector<QVector<QString> > items;

    QString project_role_id;
    if (DataBaseHandler::show_items("def_project_role", {"project_role_id", "project_role"}, items, "", "order by project_role_id") && items.size() > 0)
        project_role_id = items[0][0];
    else return false;

    bool success = true;
    success = success && DataBaseHandler::update_items("chip_chip", "owner", user_id, {{"owner", my_user_id}});
    success = success && DataBaseHandler::update_items("block_system_block", "responsible", user_id, {{"responsible", my_user_id}});

    QSet<QString> chip_ids;
    items.clear();
    success = success && DataBaseHandler::show_items("chip_chip", {"chip_id"}, "owner", my_user_id, items);
    success = success && DataBaseHandler::show_items("block_system_block", {"chip_id"}, "responsible", my_user_id, items);
    success = success && DataBaseHandler::delete_items("chip_designer", "user_id", user_id);
    for (const auto& item : items) chip_ids.insert(item[0]);

    if (!success) return false;
    for (const QString& chip_id : chip_ids)
    {
        success = success && DataBaseHandler::insert_item("chip_designer",
                                                        {"chip_id", "user_id", "project_role_id"},
                                                        {chip_id, my_user_id, project_role_id});
    }
    return success && DataBaseHandler::delete_items("global_user", "user_id", user_id);
}

bool DatabaseUtils::copy_row(const QString& tablename, const QString& id_field,
                     const QVector<QString>& data_fields,
                     const QHash<QString, QHash<QString, QString> >& in_old2news,
                     QHash<QString, QString>& out_old2new,
                     bool order, const QString& key, const QString& value)
{
    QString constraint = key +  " = \"" + value + "\"";
    return DatabaseUtils::copy_row(tablename, id_field, data_fields, in_old2news, out_old2new, order, constraint);
}

bool DatabaseUtils::copy_row(const QString& tablename, const QString& id_field,
                   const QVector<QString>& data_fields,
                   const QHash<QString, QHash<QString, QString> >& in_old2news,
                   QHash<QString, QString>& out_old2new,
                   bool order, const QVector<QPair<QString, QString>> &key_value_pairs)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (int i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return DatabaseUtils::copy_row(tablename, id_field, data_fields, in_old2news, out_old2new, order, constraint);
}

bool DatabaseUtils::copy_row(const QString& tablename, const QString& id_field,
                   const QVector<QString>& data_fields,
                   const QHash<QString, QHash<QString, QString> >& in_old2news,
                   QHash<QString, QString>& out_old2new,
                   bool order, const QString& constraint)
{
    bool success = true;
    QVector<QVector<QString> > items;
    QVector<QString> fields = data_fields;
    fields.insert(0, id_field);
    if (order)
    {
        fields.push_back("prev");
        fields.push_back("next");
    }
    success = success && DataBaseHandler::show_items(tablename, fields, items, constraint);
    if (order) items = sort_doubly_linked_list(items);
    QString id;
    DataBaseHandler::get_next_auto_increment_id(tablename, id_field, id);
    QString prev_id = "-1", next_id;

    for (const auto& item : items)
    {
        QString old_id = item[0];
        out_old2new[old_id] = id;
        next_id = QString::number(id.toInt() + 1);
        QVector<QString> values = {id};
        for (int i  = 0; i < data_fields.size(); i++)
        {
            QString data_field = data_fields[i], data = item[i + 1];
            if (in_old2news.contains(data_field))
            {
                if (in_old2news[data_field].contains(data)) data = in_old2news[data_field][data];
                else data = in_old2news[data_field]["default"];
            }
            values.push_back(data);
        }
        if (order)
        {
            values.push_back(prev_id);
            values.push_back(next_id);
        }
        success = success && DataBaseHandler::insert_item(tablename, fields, values);
        prev_id = id;
        id = next_id;
    }
    if (order) success = success && DataBaseHandler::update_items(tablename, {{id_field, prev_id}}, {{"next", "-1"}});

    return success;
}
