#ifndef REGISTER_MANAGER_H
#define REGISTER_MANAGER_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QHash>
#include <QTableWidget>
#include "authenticator.h"
#include <QSplitter>
#include "login_dialog.h"


namespace Ui {
class RegisterManager;
}

class RegisterManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit RegisterManager(QWidget *parent = nullptr);
    ~RegisterManager();

private slots:
    void on_actionUserManagement_triggered();
    void on_actionChangePassword_triggered();
    void on_actionLogOut_triggered();

    void on_actionNewChip_triggered();
    void on_actionOpenChip_triggered();
    void on_actionCloseChip_triggered();
    void on_actionChipManagement_triggered();
    void on_actionHTML_triggered();

    void on_actionDocEditorView_triggered();
    void on_actionChipEditorView_triggered();

    void on_chipNavigator_chip_clicked();
    void on_chipNavigator_block_clicked(QString block_id);
    void on_chipNavigator_register_clicked(QString block_id, QString reg_id);
    void on_chipNavigator_signal_clicked(QString block_id, QString sig_id);

    void on_chipEditorView_block_added(QString block_id, QString block_name, QString block_abbr, QString responsible);
    void on_chipEditorView_block_removed(int row);
    void on_chipEditorView_block_modified(int row, QString block_name, QString block_abbr, QString responsible);
    void on_chipEditorView_block_order_exchanged(int from, int to);
    void on_chipEditorView_to_refresh_block();

public slots:
    void on_loggedin(QString);

private:
    Ui::RegisterManager *ui;
    QString username_, user_id_, db_role_, db_role_id_, chip_, chip_id_, block_, block_id_, chip_owner_id_, chip_owner_;
    int address_width_, register_width_;
    QVector<QString> blocks_;
    Authenticator authenticator_;
    void init_db();
    void clear_db();
    void open_chip();

    bool msb_first_;
    LoginDialog login_dialog_;
};

#endif // REGISTER_MANAGER_H
