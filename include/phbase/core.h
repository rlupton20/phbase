#ifndef PHBASE_CORE_H
#define PHBASE_CORE_H

/******************************************************************************/
/* Statically determine the length of a static array.                         */
/******************************************************************************/
#define PHBASE_STATIC_ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

/******************************************************************************/
/* Statically determine the length of a static string                         */
/******************************************************************************/
#define PHBASE_STATIC_STRLEN(s) (sizeof("" s "") - 1)

/******************************************************************************/
/* Declare a parameter unused.                                                */
/******************************************************************************/
#define PHBASE_UNUSED_PARAMETER(p) ((void)p)

#endif
