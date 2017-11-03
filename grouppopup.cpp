#include "grouppopup.h"
#include "ui_grouppopup.h"

GroupPopUp::GroupPopUp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupPopUp)
{
    ui->setupUi(this);
    displayResults(ui->onlineview, "SELECT username FROM innodb.USERS where status = 1");           // populate online "friends"
}

GroupPopUp::~GroupPopUp()
{
    delete ui;
}
