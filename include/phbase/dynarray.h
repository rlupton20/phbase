#ifndef PHBASE_DYNARRAY_H
#define PHBASE_DYNARRAY_H

#include <stddef.h>

#include <phbase/status.h>

/******************************************************************************/
/* Parameterised type for a dynamic array of type ty.                         */
/******************************************************************************/
#define PHBASE_DYNARRAY

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  /****************************************************************************/
  /* (C11) Are dynamic arrays of type ty supported?                           */
  /****************************************************************************/
  #define PHBASE_DYNARRAY_TYPE_IS_SUPPORTED(ty) (!(_Alignof(ty) > _Alignof(max_align_t)))
#endif

/******************************************************************************/
/* Initialize the dynamic array pointed to by da with a given capacity.       */
/******************************************************************************/
#define phbase_dynarray_init_with_capacity(da, capacity)	\
    phbase_dynarray__init_with_capacity((void**)(da), (capacity), sizeof(**(da)))

/******************************************************************************/
/* Clear the memory associated with the dynamic array pointed to by da.       */
/******************************************************************************/
#define phbase_dynarray_clear(da)		                \
    phbase_dynarray__clear((void**)(da))

/******************************************************************************/
/* Obtain the length of the dynamic array pointed to by da.                   */
/******************************************************************************/
#define phbase_dynarray_length(da)		                \
    phbase_dynarray__length((void**)(da))

/******************************************************************************/
/* Obtain the capacity of the dynamic array pointed to by da.                 */
/******************************************************************************/
#define phbase_dynarray_capacity(da)		                \
    phbase_dynarray__capacity((void**)(da))

/******************************************************************************/
/* Push v onto the end of the dynamic array pointed to by da.                 */
/******************************************************************************/
#define phbase_dynarray_push(da, v)					\
    (phbase_dynarray__ensure_capacity((void**)(da), 1, sizeof(**(da))) == PHBASE_STATUS_OK ? \
     (*(da))[phbase_dynarray__claim_back((void**)(da))] = (v), PHBASE_STATUS_OK	\
     :									\
     PHBASE_STATUS_NO_MEMORY)

/******************************************************************************/
/* Drop the last element of the dynamic array.              .                 */
/******************************************************************************/
#define phbase_dynarray_pop(da, out)		\
    phbase_dynarray__pop((void**)(da), out, sizeof(*out))

/******************************************************************************/
/* Internal procedures.                                                       */
/******************************************************************************/
int
phbase_dynarray__init_with_capacity(void** slots, size_t size, size_t count);

void
phbase_dynarray__clear(void** slots);

size_t
phbase_dynarray__length(void** slots);

size_t
phbase_dynarray__capacity(void** slots);

int
phbase_dynarray__ensure_capacity(void** slots, size_t count, size_t size);

size_t
phbase_dynarray__claim_back(void** slots);

int
phbase_dynarray__pop(void** slots, void* out, size_t size);

#endif
