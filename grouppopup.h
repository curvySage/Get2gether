#ifndef GROUPPOPUP_H
#define GROUPPOPUP_H

#include <QDialog>
#include <QString>
#include <Qt>
#include <connection.h>

namespace Ui {
class GroupPopUp;
}

/* Class purpose:   Creates the dialog box/form used
 *                  by a user to create a group
*/
class GroupPopUp : public QDialog
{
    Q_OBJECT

public:
    QString myuser;
    bool accepted;
    connection myconn;
    explicit GroupPopUp(QWidget *parent = 0);
    ~GroupPopUp();
    void setUser(QString u);
    bool loadAddFriendsList();
    QString getNewGroupID();

private:
    Ui::GroupPopUp *ui;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
};

#endif // GROUPPOPUP_H
