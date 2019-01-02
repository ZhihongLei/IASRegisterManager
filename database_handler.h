#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H
#include <string>
#include <memory>
#include <vector>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>


class DataBaseHandler
{
public:
    DataBaseHandler(const std::string &hostname,
                    const std::string &username);
    ~DataBaseHandler();
    int create_database(const std::string &dbname);
    int delete_database(const std::string &dbname);

    int use_database(const std::string &dbname);
    int show_databases(const std::string &constraint, std::vector<std::string>& dbs);
    int create_table(const std::string &tablename,
                     const std::vector<std::string> &fields,
                     const std::vector<std::string> &datatypes,
                     const std::vector<std::string> &additionals,
                     const std::string &prikey = "");
    int delete_table(const std::string &tablename);
    int show_tables();
    int show_items(const std::string &tablename,
                   std::vector<std::vector<std::string> >& items,
                   const std::vector<std::string> &fields,
                   const std::string &constraint="");
    int show_one_item(const std::string &tablename,
                   std::vector<std::string>& items,
                   const std::vector<std::string> &fields,
                   const std::string &key,
                   const std::string &key_value);
    int delete_items(const std::string &tablename,
                   const std::string &key,
                   const std::string &key_value);
    int delete_items(const std::string &tablename,
                   const std::vector<std::string> &keys,
                   const std::vector<std::string> &key_values);
    int delete_items(const std::string &tablename,
                     const std::string &constraint="");
    int insert_item(const std::string &tablename,
                    const std::vector<std::string> &values,
                    const std::vector<std::string> *fields = nullptr);

    int alter_item(const std::string &tablename,
                   const std::vector<std::string> &fields,
                   const std::vector<std::string> &values,
                   const std::string &key,
                   const std::string &key_value);

    std::string get_error_message() {return _error_message;}

private:
    int execute(const std::string &command);
    int execute_query(const std::string &command);
    const std::string _username;
    const std::string _hostname;
    const static std::string _root_password;
    std::string _error_message;
    sql::Driver *_driver;
    std::unique_ptr< sql::Connection > _con;
    std::unique_ptr< sql::Statement > _stmt;
    std::unique_ptr< sql::ResultSet > _res;
    std::unique_ptr< sql::PreparedStatement > _pstmt;
    //
    //sql::Connection *_con;
    //sql::Statement *_stmt;
    //sql::ResultSet *_res;
    //sql::PreparedStatement *_pstmt;
};

#endif // DATABASEHANDLER_H
