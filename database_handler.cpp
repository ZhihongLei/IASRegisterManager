#include "database_handler.h"
//#include <boost/range/combine.hpp>
#include <QString>
#include <unordered_set>
#include <QDebug>
#include "global_variables.h"

const QString DataBaseHandler::_root_password = "lzh931229";

DataBaseHandler::DataBaseHandler(const QString &hostname, const QString &database):
    _database(database),
    _hostname(hostname)
{

}

DataBaseHandler::~DataBaseHandler()
{

}

int DataBaseHandler::execute(const QString &command)
{
    try {
        // TODO: do we really need to create a driver, connector... at each execution?
        _driver = get_driver_instance();
        _con.reset(_driver->connect(_hostname.toUtf8().constData(), gUser.toUtf8().constData(), gPassword.toUtf8().constData()));
        _stmt.reset(_con->createStatement());
        _stmt->execute(command.toUtf8().constData());

        return 1;

    } catch (sql::SQLException &e) {
        _error_message = e.what();
      return 0;
    }
}

int DataBaseHandler::execute_query(const QString &command)
{
    try {
        _driver = get_driver_instance();
        _con.reset(_driver->connect(_hostname.toUtf8().constData(), gUser.toUtf8().constData(), gPassword.toUtf8().constData()));
        _stmt.reset(_con->createStatement());
        _res.reset(_stmt->executeQuery(command.toUtf8().constData()));
        return 1;

    } catch (sql::SQLException &e) {
        _error_message = e.what();
      return 0;
    }
}

int DataBaseHandler::show_databases(const QString &constraint, QVector<QString>& dbs)
{
    QString command("SHOW DATABASES");
    if (constraint != "") command = command.append(" WHERE ").append(constraint);
    if (execute_query(command))
    {
        while (_res->next())
        {
            dbs.push_back(_res->getString("Database").c_str());
        }
        return 1;
    }
    return 0;
}


int DataBaseHandler::create_database(const QString &dbname)
{
    return execute(QString("CREATE DATABASE IF NOT EXISTS ").append(dbname));
}

int DataBaseHandler::delete_database(const QString &dbname)
{
    return execute(QString("DROP DATABASE IF EXISTS ").append(dbname));
}

int DataBaseHandler::create_table(const QString &tablename,
                 const QVector<QString> &fields,
                 const QVector<QString> &datatypes,
                 const QVector<QString> &additionals,
                 const QString &prikey)
{
    QString datadef = "";
    assert(fields.size() == datatypes.size() && fields.size() == additionals.size());
    for (size_t i = 0; i < fields.size(); i++)
    {
        QString data =  " " + fields[i] + " " + datatypes[i] + " " + additionals[i];
        if (i != fields.size() - 1) data += ",";
        datadef += data;
    }
    if (prikey != "") datadef += (", PRIMARY KEY (" + prikey + ")");
    datadef = "(" + datadef + ")";
    QString command = QString("CREATE TABLE IF NOT EXISTS ").append(_database + "."+ tablename + " ").append(datadef);

    return execute(command);
}

int DataBaseHandler::create_table(const QString &tablename,
                 const QVector<QVector<QString> > &table_define,
                 const QString &primary_key,
                 const QVector<QVector<QString> > *foreign_key_defs,
                 const QVector<QString> *unique_keys)
{
    QString datadef = "";
    for (size_t i = 0; i < table_define.size(); i++)
    {
        QString field = table_define[i][0], datatype = table_define[i][1], additional;
        if (table_define[i].size() == 3) additional = table_define[i][2];
        QString data = " " + field + " " + datatype + " " + additional;
        if (i != table_define.size() - 1) data += ",";
        datadef += data;
    }
    if (primary_key != "") datadef += (", PRIMARY KEY (" + table_define[0][0] + ")");
    if (foreign_key_defs)
        for (const QVector<QString>& foreign_key : *foreign_key_defs)
        {
            const QString col = foreign_key[0], ref_table = foreign_key[1], ref_col = foreign_key[2];
            datadef += (", FOREIGN KEY (" + col + ") REFERENCES " + ref_table + "(" + ref_col + ") ON DELETE RESTRICT ON UPDATE CASCADE");
        }
    if (unique_keys)
        for (QString unique_key : *unique_keys)
        {
            datadef += (", UNIQUE (" + unique_key + ")");
        }
    datadef = "(" + datadef + ") CHARSET=utf8mb4 COLLATE=utf8mb4_bin";
    QString command = QString("CREATE TABLE IF NOT EXISTS ").append(_database + "." + tablename + " ").append(datadef);
    return execute(command);
}

int DataBaseHandler::delete_table(const QString &tablename)
{
    QString command = "DROP TABLE IF EXISTS " + _database + "." + tablename;
    return execute(command);
}

int DataBaseHandler::insert_item(const QString &tablename,
                                 const QVector<QString> &fields,
                                 const QVector<QString> &values)
{
    QString columndef;
    QString valuedef;

    assert(fields.size() == values.size());
    for (size_t i = 0; i != fields.size() - 1; i++)
        columndef += (fields[i] + ", ");
    columndef += fields[fields.size() - 1];
    columndef = "( " + columndef + " )";

    for (size_t i = 0; i != values.size() - 1; i++)
        valuedef += ("\"" + values[i] + "\", ");
    valuedef += "\"" + values[values.size() - 1] + "\"";
    valuedef = "VALUES ( " + valuedef + " )";
    QString command = "INSERT INTO " + _database + "." + tablename + " "+ columndef + " " + valuedef;
    std::cout << command.toUtf8().constData() << std::endl;
    return execute(command);

}

int DataBaseHandler::update_items(const QString &tablename,
               const QVector<QPair<QString, QString>> &key_value_pairs,
                const QVector<QPair<QString, QString>> &field_value_pairs)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (size_t i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");

    QString valuedef;
    for (size_t i = 0; i != field_value_pairs.size() - 1; i++)
        valuedef += (field_value_pairs[i].first + " = \"" + field_value_pairs[i].second + "\", ");
    valuedef += (field_value_pairs[field_value_pairs.size()-1].first + " = \"" + field_value_pairs[field_value_pairs.size()-1].second + "\" ");
    QString command = "UPDATE " + _database + "." + tablename + " SET " + valuedef + " WHERE " + constraint;
    qDebug() << command.toUtf8().constData();
    return execute(command);
}


int DataBaseHandler::update_items(const QString &tablename,
               const QString& key, const QString& value,
                const QVector<QPair<QString, QString>> &field_value_pairs)
{
    return update_items(tablename, {{key, value}}, field_value_pairs);
}

int DataBaseHandler::delete_items(const QString &tablename,
                                     const QString &key,
                                     const QString &key_value)
{
    QString constraint = key + " = \"" + key_value + "\"";
    return delete_items(tablename, constraint);
}

int DataBaseHandler::delete_items(const QString &tablename,
               const QVector<QPair<QString, QString> >& key_value_pairs)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (size_t i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return delete_items(tablename, constraint);
}

int DataBaseHandler::delete_items(const QString &tablename, const QString &constraint)
{
    QString command = "DELETE FROM " + _database + "." + tablename;
    if (constraint != "") command = command.append(" WHERE ").append(constraint);
    return execute(command);
}

int DataBaseHandler::show_items(const QString& tablename,
                                const QVector<QString>& fields,
                                QVector<QVector<QString> >& items,
                                const QString& constraint,
                                const QString& additional)
{
    QString to_select;
    assert(fields.size() > 0);

    for (size_t i = 0; i < fields.size() - 1; i ++)
        to_select += (fields[i] + ", ");
    to_select += fields[fields.size()-1];

    QString command = "SELECT " + to_select + " FROM " + _database + "." + tablename;
    if (constraint != "") command = command.append(" WHERE ").append(constraint);
    if (additional != "") command += (" " + additional);
    std::cout << command.toUtf8().constData() << std::endl;
    if (execute_query(command))
    {
        while (_res->next())
        {
            //std::cout << _res->getString("Database").c_str() << std::endl;
            QVector<QString> row;
            for (const QString &field : fields)
                row.push_back(_res->getString(field.toUtf8().constData()).c_str());
            items.push_back(row);
        }
        return 1;
    }
    return 0;
}

int DataBaseHandler::show_items(const QString& tablename,
                                const QVector<QString>& fields,
                                const QString& key,
                                const QString& value,
                                QVector<QVector<QString> >& items,
                                const QString& additional)
{
    QString constraint = key +  " = \"" + value + "\"";
    return show_items(tablename, fields, items, constraint, additional);
}

int DataBaseHandler::show_items(const QString& tablename,
                                const QVector<QString>& fields,
                                const QVector<QPair<QString, QString> >& key_value_pairs,
                                QVector<QVector<QString> >& items,
                                const QString& additional)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (size_t i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return show_items(tablename, fields, items, constraint, additional);
}

int DataBaseHandler::show_items_inner_join(const QVector<QPair<QString, QString> >& table_field_pairs,
                        const QVector<QPair<QString, QString> >& equal_table_pairs,
                        const QVector<QPair<QString, QString> >& equal_field_pairs,
                        QVector<QVector<QString> >& items,
                        const QString &constraint,
                        const QString &additional)
{
    assert (equal_table_pairs.size() == equal_field_pairs.size());
    assert (equal_table_pairs.size() > 0);
    QString to_select;
    for (size_t i = 0; i < table_field_pairs.size()-1; i++)
        to_select += (_database + "." + table_field_pairs[i].first + "." + table_field_pairs[i].second + ", ");
    to_select += (_database + "." + table_field_pairs[table_field_pairs.size()-1].first + "." + table_field_pairs[table_field_pairs.size()-1].second);
    QString tables;
    tables = _database + "." + equal_table_pairs[0].first + " INNER JOIN " + _database + "." + equal_table_pairs[0].second + " ON " + \
            _database + "." + equal_table_pairs[0].first + "." + equal_field_pairs[0].first + " = " + _database + "." + equal_table_pairs[0].second + "." + equal_field_pairs[0].second;
    for (size_t i = 1; i < equal_table_pairs.size(); i++)
    {
        tables += " INNER JOIN " + _database + "." + equal_table_pairs[i].second + " ON " + \
                _database + "." + equal_table_pairs[i].first + "." + equal_field_pairs[i].first + " = " + _database + "." + equal_table_pairs[i].second + "." + equal_field_pairs[i].second;
    }

    QString command = "SELECT " + to_select + " FROM " + tables;
    if (constraint != "") command = command.append(" WHERE ").append(constraint);
    if (additional != "") command += (" " + additional);
    if (execute_query(command))
    {
        while (_res->next())
        {
            QVector<QString> row;
            for (const QPair<QString, QString>& table_field_pair : table_field_pairs)
                row.push_back(_res->getString(table_field_pair.second.toUtf8().constData()).c_str());
            items.push_back(row);
        }
        return 1;
    }
    return 0;
}


int DataBaseHandler::show_items_inner_join(const QVector<QString>& ext_fields,
                        const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& equal_table_field_pairs,
                        QVector<QVector<QString> >& items,
                        const QString &constraint,
                        const QString &additional)
{
    QString to_select;
    for (size_t i = 0; i < ext_fields.size()-1; i++)
        to_select += (ext_fields[i] + ", ");
    to_select += (ext_fields[ext_fields.size()-1]);

    QString tables;
    for (size_t i = 0; i < equal_table_field_pairs.size(); i++)
    {
        const auto& equal_table_field_pair = equal_table_field_pairs[i];
        const QString& table_1 = equal_table_field_pair.first.first;
        const QString& field_1 = equal_table_field_pair.first.second;
        const QString& table_2 = equal_table_field_pair.second.first;
        const QString& field_2 = equal_table_field_pair.second.second;
        if (i == 0)
            tables += (_database + "." + table_1 + " INNER JOIN " + _database + "." + table_2 + " ON " +  \
                    _database + "." + table_1 + "." + field_1 + " = " + _database + "." + table_2 + "." + field_2);
        else
            tables += (" INNER JOIN " + _database + "." + table_2 + " ON " +  \
                       _database + "." + table_1 + "." + field_1 + " = " + _database + "." + table_2 + "." + field_2);
    }
    QString command = "SELECT " + to_select + " FROM " + tables;

    if (constraint != "") command = command.append(" WHERE ").append(constraint);
    if (additional != "") command += (" " + additional);
    if (execute_query(command))
    {
        while (_res->next())
        {
            QVector<QString> row;
            for (int i = 1; i <= ext_fields.size(); i++)
                row.push_back(_res->getString(i).c_str());
            items.push_back(row);
        }
        return 1;
    }
    return 0;
}


int DataBaseHandler::show_items_inner_join(const QVector<QString>& ext_fields,
                        const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& table_field_pairs,
                        QVector<QVector<QString> >& items,
                        const QVector<QPair<QString, QString> >& key_value_pairs,
                        const QString &additional)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (size_t i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return show_items_inner_join(ext_fields, table_field_pairs, items, constraint, additional);

}

int DataBaseHandler::show_one_item(const QString &tablename,
               QVector<QString>& item,
               const QVector<QString> &fields,
               const QString &key,
               const QString &key_value)
{

    QString constraint = key + " = \"" + key_value + "\"";
    return show_one_item(tablename, item, fields, constraint);
}

int DataBaseHandler::show_one_item(const QString &tablename,
               QVector<QString>& item,
               const QVector<QString> &fields,
               const QVector<QPair<QString, QString> >& key_value_pairs)
{
    QString constraint = key_value_pairs[0].first +  " = \"" + key_value_pairs[0].second + "\"";
    for (size_t i = 1; i < key_value_pairs.size(); i++)
        constraint += (" AND " + key_value_pairs[i].first +  " = \"" + key_value_pairs[i].second + "\"");
    return show_one_item(tablename, item, fields, constraint);
}

int DataBaseHandler::show_one_item(const QString &tablename,
               QVector<QString>& item,
               const QVector<QString> &fields,
               const QString& constraint)
{
    QString to_select;
    assert(fields.size() > 0);

    for (size_t i = 0; i < fields.size() - 1; i ++)
        to_select += (fields[i] + ", ");
    to_select += fields[fields.size()-1];
    QString command = "SELECT " + to_select + " FROM " + _database + "." + tablename + " WHERE " + constraint;
    if (execute_query(command))
    {
        if (_res->next())
            for (const QString &field : fields)
                item.push_back(_res->getString(field.toUtf8().constData()).c_str());
        return 1;
    }
    return 0;
}

