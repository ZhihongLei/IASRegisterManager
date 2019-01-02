#include "register_manager.h"
#include "ui_register_manager.h"
#include <iostream>
#include "QComboBox"

Register_Manmager::Register_Manmager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Register_Manmager)
{
    ui->setupUi(this);
    QComboBox* combo = new QComboBox;
    combo->addItem("YES");
    combo->addItem("NO");
    ui->tableWidget->setCellWidget(0, 2, combo);
    //ui->tableWidget->setCellWidget(0, 3, new QComboBox(combo));
    //combo->currentIndex();
}

Register_Manmager::~Register_Manmager()
{
    delete ui;
}

void Register_Manmager::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    if (!item->parent())
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
    else {
        ui->stackedWidget->setCurrentIndex(1);
    }

}
