#ifndef PHBASE_EXTRUSION_H
#define PHBASE_EXTRUSION_H

#include <stddef.h> /* offsetof */

/******************************************************************************/
/* Recover a structures address from a members address.                       */
/******************************************************************************/
#define PHBASE_EXTRUDE(v, ty, member) ((ty*)((char*)v - offsetof(ty, member)))

#endif
