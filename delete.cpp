#include "error.h"
#include "delete.h"

/*=================================================================================================================================*/
//                                          class Delete-Specific Methods
/*=================================================================================================================================*/

/*.------------------------.*/
/*|      Constructor       |*/
/*'------------------------'*/

Delete::Delete(){
    //does nothing
}

/* Purpose:         Allows event deletion
 * Postcondition:   Selected event is deleted from database and removed from calendar
*/
int Delete::Do_Delete(QString ID_Param, QString matchuser, QString currentuser, bool isGroupMode, QString TheGroup, QString GroupID)
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
        error::printError(DELETE_ERROR_INVALID_MODE);

        return 3;
    }

    if(currentuser == matchuser || TheGroup == matchuser)
    {
        QMessageBox::StandardButton choice;
        choice = QMessageBox::warning(NULL, "Delete Event?", "Are you sure you want to delete your event?", QMessageBox::Yes | QMessageBox::No);

        /* If user selects Yes,
        *      Query database removing selected event
        */
        if (choice == QMessageBox::Yes) {
            QSqlQuery query_delete;

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

                // Print query status
                if(query_delete.isActive())
                {
                    qDebug().noquote() << "Deletion of Event " + ID_Param + " successful.";
                }
                else
                {
                    qDebug() << query_delete.lastError().text();
                }

                return updateEvents;
            }
            else
            {
                QSqlQuery selectMemberQ;
                QString groupMember;

                selectMemberQ.exec("SELECT username "
                                      "FROM innodb.GROUP_MEMBERS "
                                      "WHERE groupID = '" +GroupID+ "'");

                while(selectMemberQ.next())
                {
                     groupMember = selectMemberQ.value(0).toString();
                     query_delete.exec("DELETE FROM innodb.USER_EVENTS "
                                       "WHERE username = '" +groupMember+
                                       "' AND eventID = '" +ID_Param+ "'");
                }

                // Print query status
                if(query_delete.isActive())
                {
                    qDebug().noquote() << "Deletion of Event " + ID_Param + " successful.";
                }
                else
                {
                    qDebug() << query_delete.lastError().text();
                }
                return updateMemebers;
           }
        }
        else {
            qDebug().noquote() << "Deletion of Event " + ID_Param + " canceled.";
            return 3;
        }
    }
    else
    {
        // Output error message
        error::printError(DELETE_ERROR_UNAUTH_USER);
        return 3;
    }
}
