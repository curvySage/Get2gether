#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

#include "grouppopup.h"
#include "ui_grouppopup.h"

/* Purpose:         Default constructor
 * Postconditions:  New create group dialog is created
*/
GroupPopUp::GroupPopUp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupPopUp)
{
    ui->setupUi(this);
}

/* Purpose:         Default destructor
 * Postconditions:  Destroys this object
*/
GroupPopUp::~GroupPopUp()
{
    delete ui;
}

/* Purpose:         Sets current user calling dialog box
 * Postconditions:  myuser = u
*/
void GroupPopUp::setUser(QString u)
{
    myuser = u;
}

/* Purpose:         Initializes add friends list
 * Postconditions:  addFriendsList is filled with all
 *                  users (other than myuser) from database
 *                  in a checkbox format
 *                  Returns bool indicating initialization of addFriendsList
*/
bool GroupPopUp::loadAddFriendsList()
{
    QSqlQuery *query = new QSqlQuery(myconn.db);
    QString result;

    query->prepare("SELECT username AS \"User\" FROM innodb.USERS WHERE username !='" +myuser+ "'");

    /*  If query executes,
     *      Iterate through each query result
     *          Create new list item init. to username
     *          Make each new list item a checkbox
     *  Else
     *      Output last query error to console
    */
    if(query->exec())
    {
        while(query->next())
        {
            QListWidgetItem *newItem = new QListWidgetItem(query->value(0).toString(), ui->addFriendsList);     // create new list item, add to addFriendsList
            newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);                                      // make new item a checkable item
            newItem->setCheckState(Qt::Unchecked);                                                              // make it unchecked @ load
        }

        return true;
    }
    else
    {
        QMessageBox::critical(this,tr("error::"),query->lastError().text());
        return false;
    }
}

/* Purpose:         Creates new group using group name indicated
 *                  (MUST ALSO-->) and sends invites to all users
 *                  checked from add friends list
 * Postconditions:  New group name inserted into innodb.GROUPS
 *                  myuser inserted into innodb.GROUP_MEMBERS as first group
 *                      member of new group
 *                  (MUST ALSO-->) Sends invites to all checked users in addFriendsList
*/
void GroupPopUp::on_buttonBox_accepted()
{
    QSqlQuery query, memberQuery;
    QString groupName = ui->groupName->toPlainText();   // grab new group name
    QString newGroupID;                                 // to associate new group with myuser

    /* If group name is blank
     *      Don't create group
     *      Print error onto console
    */
    if(groupName.isEmpty())
    {
        qDebug() << "Error: Name not entered. Group not created.";
        return;
    }

    query.exec("INSERT INTO innodb.GROUPS(name) VALUES ('" +groupName+ "')");       // Create new group

    newGroupID = getNewGroupID();   // grab new group's groupID

    memberQuery.exec("INSERT INTO innodb.GROUP_MEMBERS(username, groupID) VALUES ('" +myuser+ "','" +newGroupID+ "')"); // Insert new entry into GROUP_MEMBERS

    /*  If queries are active
     *      Print insertion status onto console
     *  Else
     *      Print error onto consol
    */
    if (query.isActive() && memberQuery.isActive()) {
        qDebug("Inserted event into database.");
    }
    else {
        qDebug() << query.lastError().text();
        qDebug() << memberQuery.lastError().text();
    }

    // -------> MUST NOW INCLUDE... <---------
    // Identify users selected in add friends list
    // Send invites to checked users

    accepted = true;
    GroupPopUp::close();
}

/*  Purpose:        Closes dialog when Cancel is clicked
 *  Postcondtions:  Sets accepted to false and closes dialog
*/
void GroupPopUp::on_buttonBox_rejected()
{
    accepted = false;
    GroupPopUp::close();
}

/* Purpose:         Returns string value of newly created
 *                  group's ID
 * Postconditions:  groupID is returned in type QString
*/
QString GroupPopUp::getNewGroupID()
{
    QSqlQuery *query = new QSqlQuery(myconn.db);
    QString result;
    query->prepare("SELECT MAX(ID) FROM innodb.GROUPS");
    query->exec();
    query->first();

    result = query->value(0).toString();
    return result;
}
