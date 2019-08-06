#include "authenticator.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QMessageBox>

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

    QVector<QString> item;
    DataBaseHandler::show_one_item("def_db_role", item, {"add_user", "remove_user", "add_project", "remove_project", "full_access_to_all_projects"}, "db_role_id", db_role_id);
    if (item.size() == 0)
    {
        QMessageBox::warning(nullptr, "Authenticator", "Unable to retrieve database permissions.\nExceptions might happen.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    if (item[0] == "1") db_permissions_ |= DATABASE_PERMISSIONS::ADD_USER;
    if (item[1] == "1") db_permissions_ |= DATABASE_PERMISSIONS::REMOVE_USER;
    if (item[2] == "1") db_permissions_ |= DATABASE_PERMISSIONS::ADD_PROJECT;
    if (item[3] == "1") db_permissions_ |= DATABASE_PERMISSIONS::REMOVE_PROJECT;
    if (item[4] == "1") db_permissions_ |= DATABASE_PERMISSIONS::FULL_ACCESS_TO_ALL_PROJECTS;
}

void Authenticator::set_project_permissions(bool setting)
{
    project_permissions_ = 0;
    if (setting)
    {
        project_permissions_ |= PROJECT_PERMISSIONS::ADD_BLOCK;
        project_permissions_ |= PROJECT_PERMISSIONS::REMOVE_RESPONSIBLE_BLOCK;
        project_permissions_ |= PROJECT_PERMISSIONS::READ_ALL_BLOCKS;
        project_permissions_ |= PROJECT_PERMISSIONS::ADD_CHIP_DESIGNER;
        project_permissions_ |= PROJECT_PERMISSIONS::REMOVE_CHIP_DESIGNER;
        project_permissions_ |= PROJECT_PERMISSIONS::FULL_ACCESS_TO_ALL_BLOCKS;
    }
}

void Authenticator::set_project_permissions(const QString& project_role_id)
{
    project_permissions_ = 0;
    QVector<QString> item;
    DataBaseHandler::show_one_item("def_project_role", item, {"add_block", "remove_responsible_block", "read_all_blocks", "add_chip_designer", "remove_chip_designer", "full_access_to_all_blocks"}, "project_role_id", project_role_id);
    if (item.size() == 0)
    {
        QMessageBox::warning(nullptr, "Authenticator", "Unable to retrieve project permissions.\nExceptions might happen.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    if (item[0] == "1") project_permissions_ |= PROJECT_PERMISSIONS::ADD_BLOCK;
    if (item[1] == "1") project_permissions_ |= PROJECT_PERMISSIONS::REMOVE_RESPONSIBLE_BLOCK;
    if (item[2] == "1") project_permissions_ |= PROJECT_PERMISSIONS::READ_ALL_BLOCKS;
    if (item[3] == "1") project_permissions_ |= PROJECT_PERMISSIONS::ADD_CHIP_DESIGNER;
    if (item[4] == "1") project_permissions_ |= PROJECT_PERMISSIONS::REMOVE_CHIP_DESIGNER;
    if (item[5] == "1") project_permissions_ |= PROJECT_PERMISSIONS::FULL_ACCESS_TO_ALL_BLOCKS;
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
        block_permissions_ |= BLOCK_PERMISSIONS::EDIT_SIGNAL_PARTITION;
        block_permissions_ |= BLOCK_PERMISSIONS::EDIT_REGISTER_PARTITION;
        block_permissions_ |= BLOCK_PERMISSIONS::EDIT_DOCUMENT;
    }
}

void Authenticator::freeze(bool frozen)
{
    frozen_ = frozen;
}

bool Authenticator::can_add_user() const
{
    return (db_permissions_ & DATABASE_PERMISSIONS::ADD_USER) != 0;
}

bool Authenticator::can_remove_user() const
{
    return (db_permissions_ & DATABASE_PERMISSIONS::REMOVE_USER) != 0;
}

bool Authenticator::can_add_project() const
{
    return (db_permissions_ & DATABASE_PERMISSIONS::ADD_PROJECT) != 0;
}

bool Authenticator::can_remove_project() const
{
    return (db_permissions_ & DATABASE_PERMISSIONS::REMOVE_PROJECT) != 0;
}

bool Authenticator::can_fully_access_all_projects() const
{
    return (db_permissions_ & DATABASE_PERMISSIONS::FULL_ACCESS_TO_ALL_PROJECTS) != 0;
}

bool Authenticator::frozen() const
{
    return frozen_;
}

bool Authenticator::can_add_block() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::ADD_BLOCK) != 0 && !frozen_;
}

bool Authenticator::can_remove_responsible_block() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::REMOVE_RESPONSIBLE_BLOCK) != 0 && !frozen_;
}

bool Authenticator::can_read_all_blocks() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::READ_ALL_BLOCKS) != 0;
}

bool Authenticator::can_add_chip_designer() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::ADD_CHIP_DESIGNER) != 0;
}

bool Authenticator::can_remove_chip_designer() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::REMOVE_CHIP_DESIGNER) != 0;
}

bool Authenticator::can_fully_access_all_blocks() const
{
    return (project_permissions_ & PROJECT_PERMISSIONS::FULL_ACCESS_TO_ALL_BLOCKS) != 0 && !frozen_;
}

bool Authenticator::can_add_signal() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::ADD_SIGNAL) != 0 && !frozen_;
}

bool Authenticator::can_remove_signal() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::REMOVE_SIGNAL) != 0 && !frozen_;
}

bool Authenticator::can_add_register() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::ADD_REGISTER) != 0 && !frozen_;
}

bool Authenticator::can_remove_register() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::REMOVE_REGISTER) != 0 && !frozen_;
}

bool Authenticator::can_edit_signal_partition() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::EDIT_SIGNAL_PARTITION) != 0 && !frozen_;
}

bool Authenticator::can_edit_register_partition() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::EDIT_REGISTER_PARTITION) != 0 && !frozen_;
}

bool Authenticator::can_edit_document() const
{
    return (block_permissions_ & BLOCK_PERMISSIONS::EDIT_DOCUMENT) != 0 || (project_permissions_ & PROJECT_PERMISSIONS::FULL_ACCESS_TO_ALL_BLOCKS);
}

void Authenticator::clear_database_permission()
{
    db_permissions_ = 0;
}

void Authenticator::clear_project_permission()
{
    project_permissions_ = 0;
}

void Authenticator::clear_block_permission()
{
    block_permissions_ = 0;
}

void Authenticator::clear_all_permission()
{
    clear_database_permission();
    clear_project_permission();
    clear_block_permission();
}
