#include <QMessageBox>
#include <QString>
#include <QDebug>
#include "error.h"

error::error()
{

}

void error::printError(ErrorCode error_code)
{
    QMessageBox ErrorBox;
    ErrorBox.setWindowTitle("Woah There!");
    QString errorMsg, consoleMsg;

    switch(error_code)
    {
        case EDIT_ERROR_UNAUTH_USER:
        {
            errorMsg = "Error: You can't change other's schedules.";
            consoleMsg = "Modification failed: You can only modify events you own.";
            break;
        }
        case DELETE_ERROR_UNAUTH_USER:
        {
            errorMsg = "Error : You can't delete other's schedules.";
            consoleMsg = "Deletion failed: You can only delete events you own.";
            break;
        }
        case ADD_ERROR_NO_GROUP_SELECTED:
        {
            errorMsg = "Error : Select a group you want to add a group event.";
            consoleMsg = "Insertion failed : Select group to add group event.";
            break;
        }
        case EDIT_ERROR_INVALID_MODE:
        {
            errorMsg = "Error : You can't edit group event from personal calendar.";
            consoleMsg = "Modification failed : Group event must be edited from group calendar.";
            break;
        }
        case DELETE_ERROR_INVALID_MODE:
        {
            errorMsg = "Error : You can't delete group event from personal calendar.";
            consoleMsg = "Deletion failed : Group event must be deleted from group calendar.";
            break;
        }
        case DELETE_NO_DATE_SELECTED:
        {
            errorMsg = "Error : No event was selected. Try again.";
            consoleMsg = "Deletion failed : Select an event to delete.";
            break;
        }
        case TXTBOX_MAX_CHAR_REACHED:
        {
            errorMsg = "Error : Your input is over the character limit.";
            consoleMsg = "Character input failed : You must include no more characters than what's indicated.";
        }
        default:
        {
            errorMsg = "Error : action causes unknown error.";
            consoleMsg = "Recent action has caused an error";
        }
    }

    ErrorBox.setText(errorMsg);
    ErrorBox.exec();
    qDebug().noquote() << consoleMsg;
}
