#ifndef PHBASE_STATUS_H
#define PHBASE_STATUS_H

/******************************************************************************/
/* Return codes for phbase procedures.                                        */
/******************************************************************************/
enum phbase_status
{
    /* The operation completed successfully. */
    PHBASE_STATUS_OK = 0,
    /* A requested entity does not exist */
    PHBASE_STATUS_DOES_NOT_EXIST,
    /* There was insufficient memory to complete the operation. */
    PHBASE_STATUS_NO_MEMORY,
    /* The operation could not complete because of an invalid parameter. */
    PHBASE_STATUS_INVALID_PARAMETER,
    /* The operation could not complete because the entity was full. */
    PHBASE_STATUS_FULL,
    /* Upper bound for status variants */
    PHBASE_STATUS_MAX_STATUS,
};

#endif
