#ifndef PHBASE_PTRDEX_H
#define PHBASE_PTRDEX_H

#include <stddef.h>
#include <stdint.h>

/******************************************************************************/
/* Structure to imbue two arrays with a hash-table structure.                 */
/******************************************************************************/
struct phbase_ptrdex__internal
{
    void* key;          /* Pointer to array of keys.             */
    void* value;        /* Pointer to array of values.           */
    size_t used;        /* Slots of table in use.                */
    size_t capacity;    /* Total slots in table.                 */
    uint32_t* locator;  /* Array of locators (augmented hashes). */
};

/******************************************************************************/
/* Opaque structure for locating a slot for value insertion by key.           */
/******************************************************************************/
struct phbase_ptrdex_locator
{
    uint32_t locator;
    size_t slot;
};

/******************************************************************************/
/* Initialize a ptrdex-table with a minimal capacity.                         */
/******************************************************************************/
#define phbase_ptrdex_init_with_capacity(t, capacity)			       \
    phbase_ptrdex__init_with_capacity(&(t)->internal, (capacity),	       \
				      sizeof((t)->value[0]))

/******************************************************************************/
/* Clear the memory used by a ptrdex-table.                                   */
/******************************************************************************/
#define phbase_ptrdex_clear(t) phbase_ptrdex__clear(&(t)->internal)

/******************************************************************************/
/* Determine if a ptrdex-table is initialized.                                */
/******************************************************************************/
#define phbase_ptrdex_is_initialized(t)			                       \
    phbase_ptrdex__is_initialized(&(t)->internal)

/******************************************************************************/
/* Find a key slot in the ptrdex table.                                       */
/******************************************************************************/
#define phbase_ptrdex_find(t, k, out)    				       \
    phbase_ptrdex__find(&(t)->internal, PHBASE_PTRDEX__COERCE_KEY(k), out)

/******************************************************************************/
/* Obtain a locator for key-value insertion for a given key.                  */
/******************************************************************************/
#define phbase_ptrdex_obtain_slot(t, k, out)				       \
    phbase_ptrdex__obtain_slot(&(t)->internal,				       \
			       PHBASE_PTRDEX__COERCE_KEY(k), (out),	       \
			       sizeof((t)->value[0]))

/******************************************************************************/
/* Insert a key-value pair using a locator.                 .                 */
/******************************************************************************/
#define phbase_ptrdex_insert(t, l, k, v)				       \
    (t)->internal.locator[(l).slot] = (l).locator,			       \
	(t)->key[(l).slot] == (k) ? 0 : (t)->internal.used++,		       \
	(t)->key[(l).slot] = (k),					       \
	(t)->value[(l).slot] = (v)


/******************************************************************************/
/* Internal procedures and macros.                                            */
/******************************************************************************/

/******************************************************************************/
/* Coerce generic pointer types to void*. The comparison with NULL            */
/* checks that k has pointer type. Note gcc errors when k is a                */
/* function pointer (which is the ideal behaviour).                           */
/******************************************************************************/
#define PHBASE_PTRDEX__COERCE_KEY(k) ((k) != NULL ? (void*)(k) : NULL)

int
phbase_ptrdex__init_with_capacity(struct phbase_ptrdex__internal* t, size_t capacity,
				  size_t value_size);
void
phbase_ptrdex__clear(struct phbase_ptrdex__internal* t);

int
phbase_ptrdex__is_initialized(const struct phbase_ptrdex__internal* t);

int
phbase_ptrdex__obtain_slot(struct phbase_ptrdex__internal* t, void* key,
			   struct phbase_ptrdex_locator* out, size_t value_size);

int
phbase_ptrdex__find(const struct phbase_ptrdex__internal* t, void* key, size_t* out);

#endif
