#include "database_handler.h"
#include <boost/range/combine.hpp>
#include <QMessageBox>

const std::string DataBaseHandler::_root_password = "lzh931229";

DataBaseHandler::DataBaseHandler(const std::string &hostname, const std::string &username):
    _username(username),
    _hostname(hostname)
{
}
DataBaseHandler::~DataBaseHandler()
{
    //if (_driver) delete _driver;
    /*
    if (_res) delete _res;
    if (_stmt) delete _stmt;
    if (_pstmt) delete _pstmt;
    if (_con) delete _con;
    */
}

int DataBaseHandler::execute(const std::string &command)
{
    try {

        _driver = get_driver_instance();
        _con.reset(_driver->connect(_hostname, "root", _root_password));
        _stmt.reset(_con->createStatement());
        _stmt->execute(command);

        return 1;

    } catch (sql::SQLException &e) {
        _error_message = e.what();
      return 0;
    }
}

int DataBaseHandler::execute_query(const std::string &command)
{
    try {
        _driver = get_driver_instance();
        _con.reset(_driver->connect(_hostname, "root", _root_password));
        _stmt.reset(_con->createStatement());
        _res.reset(_stmt->executeQuery(command));

        return 1;

    } catch (sql::SQLException &e) {
        _error_message = e.what();
      return 0;
    }
}

int DataBaseHandler::show_databases(const std::string &constraint, std::vector<std::string>& dbs)
{
    std::string command("SHOW DATABASES");
    if (constraint != "") command = command.append(" WHERE ").append(constraint);
    //std::cout << command << std::endl;
    if (execute_query(command))
    {
        while (_res->next())
        {
            //std::cout << _res->getString("Database").c_str() << std::endl;
            dbs.push_back(_res->getString("Database").c_str());
        }
        return 1;
    }
    return 0;
}


int DataBaseHandler::create_database(const std::string &dbname)
{
    return execute(std::string("CREATE DATABASE IF NOT EXISTS ").append(dbname.c_str()));
}

int DataBaseHandler::delete_database(const std::string &dbname)
{
    return execute(std::string("DROP DATABASE IF NOT EXISTS ").append(dbname.c_str()));
}

int DataBaseHandler::create_table(const std::string &tablename,
                 const std::vector<std::string> &fields,
                 const std::vector<std::string> &datatypes,
                 const std::vector<std::string> &additionals,
                 const std::string &prikey)
{
    std::string datadef = "";
    assert(fields.size() == datatypes.size() && fields.size() == additionals.size());
    for (size_t i = 0; i < fields.size(); i++)
    {
        std::string data =  " " + fields[i] + " " + datatypes[i] + " " + additionals[i];
        if (i != fields.size() - 1) data += ",";
        datadef += data;
    }
    if (prikey != "") datadef += (", PRIMARY KEY (" + prikey + ")");
    datadef = "(" + datadef + ")";
    std::string command = std::string("CREATE TABLE IF NOT EXISTS ").append(tablename + " ").append(datadef);
    //std::cout << command << std::endl;

    return execute(command);
}

int DataBaseHandler::delete_table(const std::string &tablename)
{
    std::string command = "DROP TABLE IF EXISTS " + tablename;
    return execute(command);
}

int DataBaseHandler::insert_item(const std::string &tablename,
                                 const std::vector<std::string> &values,
                                 const std::vector<std::string> *fields)
{
    std::string columndef;
    std::string valuedef;
    if (fields)
    {
        assert(fields->size() == values.size());
        for (size_t i = 0; i != fields->size() - 1; i++)
            columndef += ((*fields)[i] + ", ");
        columndef += (*fields)[fields->size() - 1];
        columndef = "( " + columndef + " )";
    }
    for (size_t i = 0; i != values.size() - 1; i++)
        valuedef += (values[i] + ", ");
    valuedef += values[values.size() - 1];
    valuedef = "VALUES ( " + valuedef + " )";
    std::string command = "INSERT INTO " + tablename + " "+ columndef + " " + valuedef;
    //std::cout << command << std::endl;
    return execute(command);

}

int DataBaseHandler::alter_item(const std::string &tablename,
                                const std::vector<std::string> &fields,
                                const std::vector<std::string> &values,
                                const std::string &key,
                                const std::string &key_value)
{

    std::string valuedef;
    for (size_t i = 0; i != fields.size() - 1; i++)
        valuedef += (fields[i] + "=" + values[i] + ", ");
    valuedef += (fields[fields.size()-1] + " = " + values[values.size()-1] + " ");
    std::string command = "UPDATE " + tablename + " SET " + valuedef + " WHERE " + key + " = \"" + key_value + "\"";
    return execute(command);
}

int DataBaseHandler::delete_items(const std::string &tablename,
                                     const std::string &key,
                                     const std::string &key_value)
{
    std::string command = "DELETE FROM " + tablename + " WHERE " + key + " = \"" + key_value + "\"";
    return execute(command);
}

int DataBaseHandler::delete_items(const std::string &tablename,
               const std::vector<std::string> &keys,
               const std::vector<std::string> &key_values)
{
    assert(keys.size() == key_values.size());
    for (size_t i = 0; i < keys.size(); i++)
        if (!delete_items(tablename, keys[i], key_values[i])) return 0;
    return 1;
}

int DataBaseHandler::delete_items(const std::string &tablename, const std::string &constraint)
{
    std::string command = "DELETE FROM " + tablename;
    if (constraint != "") command = command.append(" WHERE ").append(constraint);
    return execute(command);
}

int DataBaseHandler::show_items(const std::string &tablename,
                                std::vector<std::vector<std::string> > &items,
                                const std::vector<std::string> &fields,
                                const std::string &constraint)
{
    std::string to_select;
    assert(fields.size() > 0);

    for (size_t i = 0; i < fields.size() - 1; i ++)
        to_select += (fields[i] + ", ");
    to_select += fields[fields.size()-1];

    std::string command = "SELECT " + to_select + " FROM " + tablename;
    if (constraint != "") command = command.append(" WHERE ").append(constraint);
    //std::cout << command << std::endl;
    if (execute_query(command))
    {
        while (_res->next())
        {
            //std::cout << _res->getString("Database").c_str() << std::endl;
            std::vector<std::string> row;
            for (std::string field : fields)
                row.push_back(_res->getString(field).c_str());
            items.push_back(row);
        }
        return 1;
    }
    return 0;
}

int DataBaseHandler::show_one_item(const std::string &tablename,
               std::vector<std::string>& items,
               const std::vector<std::string> &fields,
               const std::string &key,
               const std::string &key_value)
{
    // TODO: modify this function
    // not "one item" at all because key_value can duplicate
    std::string to_select;
    assert(fields.size() > 0);

    for (size_t i = 0; i < fields.size() - 1; i ++)
        to_select += (fields[i] + ", ");
    to_select += fields[fields.size()-1];

    std::string command = "SELECT " + to_select + " FROM " + tablename + " WHERE " + key + " = \"" + key_value + "\"";
    //std::cout << command << std::endl;
    if (execute_query(command))
    {
        while (_res->next())
        {
            for (std::string field : fields)
                items.push_back(_res->getString(field).c_str());
        }
        return 1;
    }
    return 0;
}

