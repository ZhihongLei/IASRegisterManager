#ifndef CHIP_NAVIGATOR_H
#define CHIP_NAVIGATOR_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "authenticator.h"

namespace Ui {
class ChipNavigator;
}

class ChipNavigator : public QWidget
{
    Q_OBJECT

public:
    explicit ChipNavigator(QWidget *parent = nullptr);
    ~ChipNavigator();
    void login(const QString& username_, const QString& user_id);
    void open_chip(const QString& chip, const QString& chip_id, bool msb_first);
    void close_chip();
    void set_authenticator(Authenticator* authenticator);
    void display_nagivator();

    void add_block(const QString& block_id, const QString& block_name, const QString& block_abbr, const QString& responsible);
    void remove_block(int row);
    void modify_block(int row, const QString& block_name, const QString& block_abbr, const QString& responsible);
    void change_block_order(int from, int to);
    void refresh_block();

private slots:
    void on_treeWidgetBlock_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_lineEditSearch_textChanged(const QString &arg1);


signals:
    void chip_clicked();
    void block_clicked(QString block_id);
    void register_clicked(QString block_id, QString reg_id);
    void signal_clicked(QString block_id, QString sig_id);

private:
    void refresh_block(QTreeWidgetItem* block_item);
    bool search(QTreeWidgetItem* item, const QString& s, bool visible);

    Ui::ChipNavigator *ui;
    Authenticator* authenticator_;
    QString chip_name_, chip_id_, username_, user_id_;
    bool msb_first_;
    QHash<QString, QString> block_id2abbr_, block_id2responsible_;
};

#endif // CHIP_NAVIGATOR_H
