#ifndef ERROR_H
#define ERROR_H


//error codes
typedef enum {
    EDIT_ERROR_UNAUTH_USER = -1,
    DELETE_ERROR_UNAUTH_USER = -2,
    ADD_ERROR_NO_GROUP_SELECTED = -3,
    EDIT_ERROR_INVALID_MODE = -4,
    DELETE_ERROR_INVALID_MODE = -5,
    DELETE_NO_DATE_SELECTED = -6,
    TXTBOX_MAX_CHAR_REACHED = -7
}ErrorCode;

// class purpose:   included to print error messages
//                  of application specific errors
class error
{
public:
    error();

    static void printError(ErrorCode error_num);
};

#endif // ERROR_H
