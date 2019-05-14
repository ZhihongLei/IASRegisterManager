#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QString>

class Authenticator
{
public:
    enum DATABASE_PERMISSIONS
    {
        IS_USER_MANAGER = 1 << 0,
        ADD_PROJECT = 1 << 1,
        REMOVE_PROJECT = 1 << 2
    };

    enum PROJECT_PERMISSIONS
    {
        ADD_BLOCK = 1 << 0,
        REMOVE_BLOCK = 1 << 1,
        READ_ALL_BLOCKS = 1 << 2,
        COMPILE_PROJECT = 1 << 3,
        ADD_PROJECT_USER = 1 << 4,
        REMOVE_PROJECT_USER = 1 << 5
    };

    enum BLOCK_PERMISSIONS
    {
        ADD_SIGNAL = 1 << 0,
        REMOVE_SIGNAL = 1 << 1,
        ADD_REGISTER = 1 << 2,
        REMOVE_REGISTER = 1 << 3,
        EDIT_DOCUMENT = 1 << 4
    };

    Authenticator(const QString& db_role_id, const QString& project_role_id);
    Authenticator();

    bool is_user_manager() const;
    bool can_add_project() const;
    bool can_remove_project() const;
    bool can_add_block() const;
    bool can_remove_block() const;
    bool can_read_all_blocks() const;
    bool can_compile_project() const;
    bool can_add_chip_designer() const;
    bool can_remove_chip_designer() const;
    bool can_add_signal() const;
    bool can_remove_signal() const;
    bool can_add_register() const;
    bool can_remove_register() const;
    bool can_edit_document() const;

    void set_database_permissions(const QString& db_role_id);
    void set_project_permissions(const QString& project_role_id);
    void set_block_permissions(bool setting);

private:
    int db_permissions_ = 0, project_permissions_ = 0, block_permissions_;
};

#endif // AUTHENTICATOR_H
