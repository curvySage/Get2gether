#include "grouppopup.h"
#include "ui_grouppopup.h"

GroupPopUp::GroupPopUp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupPopUp)
{
    ui->setupUi(this);
}

GroupPopUp::~GroupPopUp()
{
    delete ui;
}
