#ifndef REGISTER_MANAGER_H
#define REGISTER_MANAGER_H

#include <QMainWindow>


class Authenticator;
class LoginDialog;
class QCompleter;
namespace Ui {
class RegisterManager;
}

class RegisterManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit RegisterManager(QWidget *parent = nullptr);
    ~RegisterManager();
    static bool initialize();

private slots:
    void on_actionUserManagement_triggered();
    void on_actionChangePassword_triggered();
    void on_actionLogOut_triggered();

    void on_actionNewChip_triggered();
    void on_actionNewChipFrom_triggered();
    void on_actionOpenChip_triggered();
    void on_actionCloseChip_triggered();
    void on_actionFreezeChip_triggered();
    void on_actionNaming_triggered();
    void on_actionResourcesBaseDir_triggered();
    void on_actionChipManagement_triggered();

    void on_actionDocument_triggered();
    void on_actionSPISourceCode_triggered();

    void on_actionDocEditor_triggered();
    void on_actionDocPreview_triggered();
    void on_actionChipEditorView_triggered();

    void on_chipNavigator_chip_clicked();
    void on_chipNavigator_block_clicked(QString block_id);
    void on_chipNavigator_register_clicked(QString block_id, QString reg_id);
    void on_chipNavigator_signal_clicked(QString block_id, QString sig_id);

    void on_chipEditorView_chip_basics_edited(QString chip_name, QString chip_owner, QString chip_owner_id, int register_width, int address_width, bool msb_first);
    void on_chipEditorView_block_added(QString block_id, QString block_name, QString block_abbr, QString responsible);
    void on_chipEditorView_block_removed(int row);
    void on_chipEditorView_block_modified(int row, QString block_name, QString block_abbr, QString responsible);
    void on_chipEditorView_block_order_exchanged(int from, int to);
    void on_chipEditorView_to_refresh_navigator_block();

    static void LogMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

public slots:
    void on_loggedin(QString);

private:
    void open_chip();
    void set_completer();

    Ui::RegisterManager *ui;
    LoginDialog* login_dialog_;
    Authenticator* authenticator_;
    QCompleter *completer_;
    QString username_, user_id_, db_role_, db_role_id_, chip_name_, chip_id_, chip_owner_id_, chip_owner_;
    QString current_block_id_, current_reg_id_, current_sig_id_;
    int address_width_, register_width_;
    bool msb_first_;
    bool chip_opened_;
};

#endif // REGISTER_MANAGER_H
