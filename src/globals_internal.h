#ifndef GLOBALS_INTERNAL
#define GLOBALS_INTERNAL


#include "../include/globals.h"
#include "../include/string_pool.h"

#include <stdint.h>


/* Return current global ID for given string ID or allocate a new one. Note:
 * This does not define the variable, globals_define uses this ID to define
 * the variable. */
uint32_t globals_xget_global_id(struct globals *, struct str_ref str_ref);


#endif
