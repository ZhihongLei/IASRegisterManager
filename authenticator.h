#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QString>

class Authenticator
{
public:
    enum DATABASE_PERMISSIONS
    {
        ADD_USER = 1 << 0,
        REMOVE_USER = 1 << 1,
        ADD_PROJECT = 1 << 2,
        REMOVE_PROJECT = 1 << 3
    };

    enum PROJECT_PERMISSIONS
    {
        ADD_BLOCK = 1 << 0,
        REMOVE_HIS_BLOCK = 1 << 1,
        READ_ALL_BLOCKS = 1 << 2,
        COMPILE_PROJECT = 1 << 3,
        ADD_PROJECT_USER = 1 << 4,
        REMOVE_PROJECT_USER = 1 << 5,
        FULL_ACCESS_TO_ALL_BLOCKS = 1 << 6
    };

    enum BLOCK_PERMISSIONS
    {
        ADD_SIGNAL = 1 << 0,
        REMOVE_SIGNAL = 1 << 1,
        ADD_REGISTER = 1 << 2,
        REMOVE_REGISTER = 1 << 3,
        EDIT_SIGNAL_PARTITION = 1 << 4,
        EDIT_REGISTER_PARTITION = 1 << 5,
        EDIT_DOCUMENT = 1 << 6
    };

    Authenticator(const QString& db_role_id, const QString& project_role_id);
    Authenticator();

    bool can_add_user() const;
    bool can_remove_user() const;
    bool can_add_project() const;
    bool can_remove_project() const;
    bool can_add_block() const;
    bool can_remove_his_block() const;
    bool can_read_all_blocks() const;
    bool can_compile_project() const;
    bool can_add_chip_designer() const;
    bool can_remove_chip_designer() const;
    bool can_fully_access_all_blocks() const;
    bool can_add_signal() const;
    bool can_remove_signal() const;
    bool can_add_register() const;
    bool can_remove_register() const;
    bool can_edit_signal_partition() const;
    bool can_edit_register_partition() const;
    bool can_edit_document() const;

    void set_database_permissions(const QString& db_role_id);
    void set_project_permissions(const QString& project_role_id);
    void set_block_permissions(bool setting);

    void clear_database_permission();
    void clear_project_permission();
    void clear_block_permission();
    void clear_all_permission();

private:
    int db_permissions_ = 0, project_permissions_ = 0, block_permissions_;
};

#endif // AUTHENTICATOR_H
