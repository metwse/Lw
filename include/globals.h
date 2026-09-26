#ifndef GLOBALS_H
#define GLOBALS_H

#include "string_pool.h"  // IWYU pragma: keep, global_id_to_str_ref
#include "value.h"

#include <stdbool.h>
#include <stdint.h>


#define T uint32_t, struct val, global_vals
#include "../vendor/libfun/include/hmap.h"

#define T uint32_t, uint32_t, str_id_to_global_id
#include "../vendor/libfun/include/hmap.h"

#define T uint32_t, struct str_ref, global_id_to_str_ref
#include "../vendor/libfun/include/hmap.h"

#define T uint32_t, bool, undefined_globals
#include "../vendor/libfun/include/hmap.h"

#define T uint32_t, recycle_global_ids
#include "../vendor/libfun/include/stack.h"


/* Global variables. */
struct globals {
	/* global_id -> value */
	struct fhmap_global_vals global_vals;
	/* str_id -> global_id */
	struct fhmap_str_id_to_global_id m1;
	/* global_id -> str_ref */
	struct fhmap_global_id_to_str_ref m2;
	/* global_id -> is marked
	 * Map of declared but undefined globals */
	struct fhmap_undefined_globals m3;

	/* Reassign previously released ID's instead of incrementing last_id. */
	struct fstack_recycle_global_ids recycle_global_ids;
	uint32_t last_id;
};


/* Creates a new global variable table. */
void globals_xinit(struct globals *);

/* Releases resourced owned by the global table. */
void globals_destroy(struct globals *);

/* Defines a global variable. Note: The variable should not exist. */
void globals_define(struct globals *, uint32_t global_id, struct val v);

/* Gets a global variable. */
struct val *globals_get(struct globals *, uint32_t global_id);

/* Removes a global variable, returns true if a value is found and removed. */
bool globals_remove(struct globals *, uint32_t global_id, struct val *out_val);


#endif
