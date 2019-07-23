#include "database_handler.h"
#include <QString>
#include <unordered_set>
#include <QDebug>
#include "global_variables.h"
#include "data_utils.h"

sql::Driver* DataBaseHandler::driver_ = get_driver_instance();
std::unique_ptr< sql::Connection > DataBaseHandler::con_ = std::unique_ptr< sql::Connection >();
std::unique_ptr< sql::Statement > DataBaseHandler::stmt_ = std::unique_ptr< sql::Statement >();
std::unique_ptr< sql::ResultSet > DataBaseHandler::res_ = std::unique_ptr< sql::ResultSet >();
QString DataBaseHandler::error_message_;
QString DataBaseHandler::database_;


bool DataBaseHandler::initialize(const QString &hostname, const QString &database, const QString& username, const QString& password)
{
    try {
        con_.reset(driver_->connect(hostname.toUtf8().constData(), username.toUtf8().constData(), password.toUtf8().constData()));
        con_->setAutoCommit(false);
        con_->setSchema(database.toUtf8().constData());
        stmt_.reset(con_->createStatement());
        database_ = database;
        return true;
    } catch (sql::SQLException &e) {
        error_message_ = e.what();
        return false;
    }
}

bool DataBaseHandler::use_database(const QString &database)
{
    try {
        con_->setSchema(database.toUtf8().constData());
        database_ = database;
        return true;
    } catch (sql::SQLException &e) {
        error_message_ = e.what();
        return false;
    }
}

void DataBaseHandler::close()
{
    if(con_) con_->close();
}

void DataBaseHandler::commit()
{
    con_->commit();
}

void DataBaseHandler::rollback()
{
    con_->rollback();
}

bool DataBaseHandler::create_database(const QString &dbname)
{
    return execute("CREATE DATABASE " + dbname);
}

bool DataBaseHandler::delete_database(const QString &dbname)
{
    return execute("DROP DATABASE " + dbname);
}

bool DataBaseHandler::show_databases(const QString &constraint, QVector<QString>& dbs)
{
    QString statement("SHOW DATABASES");
    if (constraint != "") statement += " WHERE " + constraint;
    if (execute_query(statement))
    {
        while (res_->next())
        {
            dbs.push_back(res_->getString("Database").c_str());
        }
        return true;
    }
    return false;
}

bool DataBaseHandler::create_table(const QString &tablename,
                 const QVector<QVector<QString> > &field_definitions,
                 const QString &primary_key)
{
    QString table_definition = "";
    for (int i = 0; i < field_definitions.size(); i++)
    {
        QString field = field_definitions[i][0], datatype = field_definitions[i][1], additional;
        if (field_definitions[i].size() == 3) additional = field_definitions[i][2];
        QString data = " " + field + " " + datatype + " " + additional;
        if (i != field_definitions.size() - 1) data += ",";
        table_definition += data;
    }
    if (primary_key != "") table_definition += (", PRIMARY KEY (" + field_definitions[0][0] + ")");
    table_definition = "(" + table_definition + ") CHARSET=utf8mb4 COLLATE=utf8mb4_bin";
    QString statement = "CREATE TABLE " +  tablename + " " + table_definition;
    return execute(statement);
}

bool DataBaseHandler::delete_table(const QString &tablename)
{
    QString statement = "DROP TABLE " + tablename;
    return execute(statement);
}

bool DataBaseHandler::add_unique_key_constraint(const QString &tablename, const QVector<QString> &fields)
{
    QString key;
    for (int i = 0; i != fields.size() - 1; i++)
        key += (fields[i] + ", ");
    key += fields[fields.size() - 1];
    key = "( " + key + " )";
    QString statement = "ALTER TABLE " + tablename + " ADD UNIQUE " + key;
    return execute(statement);
}

bool DataBaseHandler::add_foreign_key_constraint(const QString &from_table, const QString &from_field, const QString &to_table, const QString &to_field)
{
    QString statement = "ALTER TABLE " + from_table + " ADD FOREIGN KEY (" + from_field + ") REFERENCES " + to_table + "(" + to_field + ") ON DELETE RESTRICT ON UPDATE CASCADE";
    return execute(statement);
}

bool DataBaseHandler::insert_item(const QString &tablename,
                                 const QVector<QString> &fields,
                                 const QVector<QString> &values)
{
    QString field_definition;
    QString value_definition;

    assert(fields.size() == values.size());
    for (int i = 0; i != fields.size() - 1; i++)
        field_definition += (fields[i] + ", ");
    field_definition += fields[fields.size() - 1];
    field_definition = "( " + field_definition + " )";

    for (int i = 0; i != values.size() - 1; i++)
        value_definition += ("\"" + values[i] + "\", ");
    value_definition += "\"" + values[values.size() - 1] + "\"";
    value_definition = "VALUES ( " + value_definition + " )";
    QString statement = "INSERT INTO " + tablename + " "+ field_definition + " " + value_definition;
    return execute(statement);
}

bool DataBaseHandler::update_items(const QString &tablename,
               const QVector<QPair<QString, QString>> &key_value_pairs,
                const QVector<QPair<QString, QString>> &field_value_pairs)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (int i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");

    QString value_definition;
    for (int i = 0; i != field_value_pairs.size() - 1; i++)
        value_definition += (field_value_pairs[i].first + " = \"" + field_value_pairs[i].second + "\", ");
    value_definition += (field_value_pairs[field_value_pairs.size()-1].first + " = \"" + field_value_pairs[field_value_pairs.size()-1].second + "\" ");
    QString statement = "UPDATE " + tablename + " SET " + value_definition + " WHERE " + constraint;
    return execute(statement);
}

bool DataBaseHandler::update_items(const QString &tablename,
               const QString& key, const QString& value,
                const QVector<QPair<QString, QString>> &field_value_pairs)
{
    return update_items(tablename, {{key, value}}, field_value_pairs);
}

bool DataBaseHandler::delete_items(const QString &tablename,
                                     const QString &key,
                                     const QString &value)
{
    QString constraint = key + " = \"" + value + "\"";
    return delete_items(tablename, constraint);
}

bool DataBaseHandler::delete_items(const QString &tablename,
               const QVector<QPair<QString, QString> >& key_value_pairs)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (int i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return delete_items(tablename, constraint);
}

bool DataBaseHandler::delete_items(const QString &tablename, const QString &constraint)
{
    QString statement = "DELETE FROM " + tablename;
    if (constraint != "") statement += " WHERE " + constraint;
    return execute(statement);
}

bool DataBaseHandler::show_items(const QString& tablename,
                                const QVector<QString>& fields,
                                QVector<QVector<QString> >& items,
                                const QString& constraint,
                                const QString& additional)
{
    QString to_select;
    assert(fields.size() > 0);

    for (int i = 0; i < fields.size() - 1; i ++)
        to_select += (fields[i] + ", ");
    to_select += fields[fields.size()-1];

    QString statement = "SELECT " + to_select + " FROM " + tablename;
    if (constraint != "") statement += " WHERE " + constraint;
    if (additional != "") statement += (" " + additional);
    if (execute_query(statement))
    {
        while (res_->next())
        {
            QVector<QString> row;
            for (const QString &field : fields)
                row.push_back(res_->getString(field.toUtf8().constData()).c_str());
            items.push_back(row);
        }
        return true;
    }
    return false;
}

bool DataBaseHandler::show_items(const QString& tablename,
                                const QVector<QString>& fields,
                                const QString& key,
                                const QString& value,
                                QVector<QVector<QString> >& items,
                                const QString& additional)
{
    QString constraint = key +  " = \"" + value + "\"";
    return show_items(tablename, fields, items, constraint, additional);
}

bool DataBaseHandler::show_items(const QString& tablename,
                                const QVector<QString>& fields,
                                const QVector<QPair<QString, QString> >& key_value_pairs,
                                QVector<QVector<QString> >& items,
                                const QString& additional)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (int i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return show_items(tablename, fields, items, constraint, additional);
}

bool DataBaseHandler::show_items_inner_join(const QVector<QString>& extended_fields,
                        const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& equal_table_field_pairs,
                        QVector<QVector<QString> >& items,
                        const QString& key,
                        const QString& value,
                        const QString &additional)
{
    QString constraint = key +  " = \"" + value + "\"";
    return show_items_inner_join(extended_fields, equal_table_field_pairs, items, constraint, additional);
}

bool DataBaseHandler::show_items_inner_join(const QVector<QString>& extended_fields,
                        const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& equal_table_field_pairs,
                        QVector<QVector<QString> >& items,
                        const QString &constraint,
                        const QString &additional)
{
    QString to_select;
    for (int i = 0; i < extended_fields.size()-1; i++)
        to_select += (extended_fields[i] + ", ");
    to_select += (extended_fields[extended_fields.size()-1]);

    QString tables;
    for (int i = 0; i < equal_table_field_pairs.size(); i++)
    {
        const auto& equal_table_field_pair = equal_table_field_pairs[i];
        const QString& table_1 = equal_table_field_pair.first.first;
        const QString& field_1 = equal_table_field_pair.first.second;
        const QString& table_2 = equal_table_field_pair.second.first;
        const QString& field_2 = equal_table_field_pair.second.second;
        if (i == 0)
            tables += (table_1 + " INNER JOIN " + table_2 + " ON " +  \
                    table_1 + "." + field_1 + " = " + table_2 + "." + field_2);
        else
            tables += (" INNER JOIN " + table_2 + " ON " +  \
                       table_1 + "." + field_1 + " = " + table_2 + "." + field_2);
    }
    QString statement = "SELECT " + to_select + " FROM " + tables;
    qDebug() << statement.toUtf8().constData();

    if (constraint != "") statement += " WHERE " + constraint;
    if (additional != "") statement += (" " + additional);
    if (execute_query(statement))
    {
        while (res_->next())
        {
            QVector<QString> row;
            for (int i = 1; i <= extended_fields.size(); i++)
                row.push_back(res_->getString(static_cast<uint32_t>(i)).c_str());
            items.push_back(row);
        }
        return true;
    }
    return false;
}


bool DataBaseHandler::show_items_inner_join(const QVector<QString>& extended_fields,
                        const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& equal_table_field_pairs,
                        QVector<QVector<QString> >& items,
                        const QVector<QPair<QString, QString> >& key_value_pairs,
                        const QString &additional)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (int i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");

    return show_items_inner_join(extended_fields, equal_table_field_pairs, items, constraint, additional);

}

bool DataBaseHandler::show_one_item(const QString &tablename,
               QVector<QString>& item,
               const QVector<QString> &fields,
               const QString &key,
               const QString &value)
{

    QString constraint = key + " = \"" + value + "\"";
    return show_one_item(tablename, item, fields, constraint);
}

bool DataBaseHandler::show_one_item(const QString &tablename,
               QVector<QString>& item,
               const QVector<QString> &fields,
               const QVector<QPair<QString, QString> >& key_value_pairs)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (int i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return show_one_item(tablename, item, fields, constraint);
}

bool DataBaseHandler::show_one_item(const QString &tablename,
               QVector<QString>& item,
               const QVector<QString> &fields,
               const QString& constraint)
{
    QString to_select;
    assert(fields.size() > 0);

    for (int i = 0; i < fields.size() - 1; i ++)
        to_select += (fields[i] + ", ");
    to_select += fields[fields.size()-1];
    QString statement = "SELECT " + to_select + " FROM " + tablename;
    if (constraint != "") statement += " WHERE " + constraint;
    if (execute_query(statement))
    {
        if (res_->next())
            for (const QString &field : fields)
                item.push_back(res_->getString(field.toUtf8().constData()).c_str());
        return true;
    }
    return false;
}

QString DataBaseHandler::get_error_message()
{
    return error_message_;
}

bool DataBaseHandler::get_next_auto_increment_id(const QString &tablename, const QString& id_field, QString &id)
{
    QVector<QString> item;
    bool success = show_one_item(tablename, item, {"max("+id_field+")"}, "");
    if (!success) return false;
    id = QString::number(item[0].toInt() + 1);
    return true;
}

bool DataBaseHandler::remove_signal(const QString& sig_id, const QString& reg_sig_id)
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

bool DataBaseHandler::remove_register(const QString &reg_id)
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

bool DataBaseHandler::remove_block(const QString &block_id)
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
        success = success && DataBaseHandler::remove_signal(sig_id, reg_sig.size() > 0 ? reg_sig[0] : "");
    }

    items.clear();
    success = success && DataBaseHandler::show_items("block_register", {"reg_id"}, "block_id", block_id, items);
    for(const auto& item: items) success = success && DataBaseHandler::remove_register(item[0]);

    success = success && DataBaseHandler::delete_items("block_system_block", "block_id", block_id);
    if (prev != "-1") success = success && DataBaseHandler::update_items("block_system_block", {{"block_id", prev}}, {{"next", next}});
    if (next != "-1") success = success && DataBaseHandler::update_items("block_system_block", {{"block_id", next}}, {{"prev", prev}});

    return success;
}

bool DataBaseHandler::remove_chip(const QString &chip_id)
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
    for (const auto& item : items) success = success && DataBaseHandler::remove_block(item[0]);

    success = success && DataBaseHandler::delete_items("chip_chip", "chip_id", chip_id);

    return success;
}

bool DataBaseHandler::remove_user(const QString& my_user_id, const QString &user_id)
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

bool DataBaseHandler::copy_row(const QString& tablename, const QString& id_field,
                     const QVector<QString>& data_fields,
                     const QHash<QString, QHash<QString, QString> >& in_old2news,
                     QHash<QString, QString>& out_old2new,
                     bool order, const QString& key, const QString& value)
{
    QString constraint = key +  " = \"" + value + "\"";
    return DataBaseHandler::copy_row(tablename, id_field, data_fields, in_old2news, out_old2new, order, constraint);
}

bool DataBaseHandler::copy_row(const QString& tablename, const QString& id_field,
                   const QVector<QString>& data_fields,
                   const QHash<QString, QHash<QString, QString> >& in_old2news,
                   QHash<QString, QString>& out_old2new,
                   bool order, const QVector<QPair<QString, QString>> &key_value_pairs)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (int i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return DataBaseHandler::copy_row(tablename, id_field, data_fields, in_old2news, out_old2new, order, constraint);
}

bool DataBaseHandler::copy_row(const QString& tablename, const QString& id_field,
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
    get_next_auto_increment_id(tablename, id_field, id);
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

bool DataBaseHandler::execute(const QString &statement)
{
    try {
        stmt_->execute(statement.toUtf8().constData());
        return true;
    } catch (sql::SQLException &e) {
        error_message_ = e.what();
      return false;
    }
}

bool DataBaseHandler::execute_query(const QString &statement)
{
    try {
        res_.reset(stmt_->executeQuery(statement.toUtf8().constData()));
        return true;
    } catch (sql::SQLException &e) {
        error_message_ = e.what();
      return false;
    }
}
