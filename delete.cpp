#include "delete.h"

Delete::Delete(){
    //does nothing
}

/* Purpose:         Allows event deletion
 * Postcondition:   Selected event is deleted from database and removed from calendar
*/
int Delete::Do_Delete(QString ID_Param, QString matchuser, QString currentuser, bool isGroupMode, QDate currDate, QString TheGroup, QString GroupID)
{
    int updateEvents = 1;
    int updateMemebers = 2;
    bool isGroupEvent;                                          // to determine if group event
    QSqlQuery isGroupEventQ;                                    // To select groupID of returned event
    isGroupEventQ.exec("SELECT groupID "
                       "FROM innodb.EVENTS "
                       "WHERE ID = '" + ID_Param+ "'");
    isGroupEventQ.first();
    isGroupEvent = (isGroupEventQ.value(0).toInt() != 0);       // Determine if group event (i.e. groupID != 0)

    /* If event is a group event but in personal mode,
     *      Output error
    */
    if(isGroupEvent && !isGroupMode)
    {
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Woah There!");
        MsgBox.setText("Sorry, but you can't delete group event from personal calendar!");
        MsgBox.exec();

        qDebug("Deletion failed : group event must be deleted from group calendar.");

        return 3;
    }

    /* If logged in user matches event owner
     *      Delete event
     * Else
     *      Print error
    */
    if(currentuser == matchuser || TheGroup == matchuser)
    {
        //QMessageBox::StandardButton choice;
        QMessageBox::warning(NULL, "Delete Event?", "Are you sure you want to delete your event?", QMessageBox::Yes, QMessageBox::No);

        /* If user selects Yes,
        *      Query database removing selected event
        */
        if (QMessageBox::Yes) {
            QSqlQuery query_delete, query_count;
            //QDate currDate = ui->dateEdit->date();
            //int eventCount;

            /* First, delete USER_EVENT eventID fk reference:
             * If not in group mode,
             *      Delete user event entry regularly
             * Else,
             *      Cycle through each group member
             *          Delete user event entry
            */
            if(!isGroupMode)
            {
                // update USER_EVENTS table
                query_delete.exec("DELETE FROM innodb.USER_EVENTS "
                                  "WHERE eventID ='" +ID_Param+
                                  "' AND username ='" +currentuser+ "'");
                //updateEventsView();
                return updateEvents;
            }
            else
            {
                QSqlQuery selectMemberQ;
                QString groupMember;

                selectMemberQ.prepare("SELECT username "
                                      "FROM innodb.GROUP_MEMBERS "
                                      "WHERE groupID = '" +GroupID+ "'");

                if(selectMemberQ.exec() && selectMemberQ.first())
                {
                    do
                    {
                     groupMember = selectMemberQ.value(0).toString();
                     query_delete.exec("DELETE FROM innodb.USER_EVENTS "
                                       "WHERE username = '" +groupMember+
                                       "' AND eventID = '" +ID_Param+ "'");
                    }while(selectMemberQ.next());
                }

                // Print query status
                if(query_delete.isActive())
                {
                    qDebug("Deletion successful.");
                }
                else
                {
                    qDebug() << query_delete.lastError().text();
                }
               // updateMemberEvents();
                return updateMemebers;
           }
        }
    }
    else
    {
        // Output error message
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Woah There!");
        MsgBox.setText("Sorry, but you can't delete other's schedules!");
        MsgBox.exec();
        return 3;
    }
}

