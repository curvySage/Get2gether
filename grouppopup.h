#ifndef GROUPPOPUP_H
#define GROUPPOPUP_H

#include <QDialog>
#include <QString>
#include <connection.h>

namespace Ui {
class GroupPopUp;
}

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

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::GroupPopUp *ui;
};

#endif // GROUPPOPUP_H
