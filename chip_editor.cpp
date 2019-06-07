#include "chip_editor.h"
#include "ui_chip_editor.h"

ChipEditor::ChipEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChipEditor)
{
    ui->setupUi(this);
}

ChipEditor::~ChipEditor()
{
    delete ui;
}
