#include "invitepopup.h"
#include "ui_invitepopup.h"

InvitePopUp::InvitePopUp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InvitePopUp)
{
    ui->setupUi(this);
}

InvitePopUp::~InvitePopUp()
{
    delete ui;
}
