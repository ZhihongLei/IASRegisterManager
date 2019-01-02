#ifndef REGISTER_MANAGER_H
#define REGISTER_MANAGER_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <string>

namespace Ui {
class Register_Manmager;
}

class Register_Manmager : public QMainWindow
{
    Q_OBJECT

public:
    explicit Register_Manmager(QWidget *parent = nullptr);
    ~Register_Manmager();

private slots:
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::Register_Manmager *ui;
};

#endif // REGISTER_MANAGER_H
