#include "authenticator.h"
#include "global_variables.h"
#include "database_handler.h"

Authenticator::Authenticator(const QString& db_role_id, const QString& project_role_id)
{
    set_database_permissions(db_role_id);
    set_project_permissions(project_role_id);

}

Authenticator::Authenticator()
{

}

void Authenticator::set_database_permissions(const QString &db_role_id)
{
    db_permissions_ = 0;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    dbhandler.show_one_item("def_db_role", item, {"is_user_manager", "add_project", "remove_project"}, "db_role_id", db_role_id);
    if (item.size() == 0) return;
    if (item[0] == "1") db_permissions_ |= DATABASE_PERMISSIONS::IS_USER_MANAGER;
    if (item[1] == "1") db_permissions_ |= DATABASE_PERMISSIONS::ADD_PROJECT;
    if (item[2] == "1") db_permissions_ |= DATABASE_PERMISSIONS::REMOVE_PROJECT;
}

void Authenticator::set_project_permissions(const QString& project_role_id)
{
    project_permissions_ = 0;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    dbhandler.show_one_item("def_project_role", item, {"add_block", "remove_block", "read_all_blocks", "compile_project", "add_chip_designer", "remove_chip_designer"}, "project_role_id", project_role_id);
    if (item[0] == "1") project_permissions_ |= PROJECT_PERMISSIONS::ADD_BLOCK;
    if (item[1] == "1") project_permissions_ |= PROJECT_PERMISSIONS::REMOVE_BLOCK;
    if (item[2] == "1") project_permissions_ |= PROJECT_PERMISSIONS::READ_ALL_BLOCKS;
    if (item[3] == "1") project_permissions_ |= PROJECT_PERMISSIONS::COMPILE_PROJECT;
    if (item[4] == "1") project_permissions_ |= PROJECT_PERMISSIONS::ADD_PROJECT_USER;
    if (item[5] == "1") project_permissions_ |= PROJECT_PERMISSIONS::REMOVE_PROJECT_USER;
}

void Authenticator::set_block_permissions(bool setting)
{
    block_permissions_ = 0;
    if (setting)
    {
        block_permissions_ |= BLOCK_PERMISSIONS::ADD_SIGNAL;
        block_permissions_ |= BLOCK_PERMISSIONS::REMOVE_SIGNAL;
        block_permissions_ |= BLOCK_PERMISSIONS::ADD_REGISTER;
        block_permissions_ |= BLOCK_PERMISSIONS::REMOVE_REGISTER;
        block_permissions_ |= BLOCK_PERMISSIONS::EDIT_DOCUMENT;
    }
}

bool Authenticator::is_user_manager() const
{
    return (db_permissions_ & DATABASE_PERMISSIONS::IS_USER_MANAGER) != 0;
}

bool Authenticator::can_add_project() const
{
    return (db_permissions_ & DATABASE_PERMISSIONS::ADD_PROJECT) != 0;
}

bool Authenticator::can_remove_project() const
{
    return (db_permissions_ & DATABASE_PERMISSIONS::REMOVE_PROJECT) != 0;
}

bool Authenticator::can_add_block() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::ADD_BLOCK) != 0;
}

bool Authenticator::can_remove_block() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::REMOVE_BLOCK) != 0;
}

bool Authenticator::can_read_all_blocks() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::READ_ALL_BLOCKS) != 0;
}

bool Authenticator::can_compile_project() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::COMPILE_PROJECT) != 0;
}

bool Authenticator::can_add_chip_designer() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::ADD_PROJECT_USER) != 0;
}

bool Authenticator::can_remove_chip_designer() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::REMOVE_PROJECT_USER) != 0;
}

bool Authenticator::can_add_signal() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::ADD_SIGNAL) != 0;
}

bool Authenticator::can_remove_signal() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::REMOVE_SIGNAL) != 0;
}

bool Authenticator::can_add_register() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::ADD_REGISTER) != 0;
}

bool Authenticator::can_remove_register() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::REMOVE_REGISTER) != 0;
}

bool Authenticator::can_edit_document() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::EDIT_DOCUMENT) != 0;
}
