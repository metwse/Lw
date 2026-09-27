#ifndef STR_POOL_H
#define STR_POOL_H

#include <stddef.h>
#include <stdint.h>


/* Underlying string. */
struct str_pool_string {
	size_t id;
	size_t ref_count;
};

#define T char, struct str_pool_string, str_pool_strings
#include "../vendor/libfun/include/hmap.h"

#define T size_t, struct fhmap_str_pool_strings_entry_mut, str_pool_rev_map
#include "../vendor/libfun/include/hmap.h"


/* String deduplication pool. */
struct str_pool {
	/* Hashmaps for various length of strings. */
	struct {
		/* chars to { string ID, reference count } map */
		struct fhmap_str_pool_strings strings;
		/* string ID to { char, string ID, reference count } map */
		struct fhmap_str_pool_rev_map rev;
	} maps[3];

	uint32_t last_id;
};

/* String pool reference. */
struct str_ref {
	struct str_pool *p;
	size_t id;
};


/* Initializes a new string pool. */
void str_pool_xinit(struct str_pool *);

/* Release resources owned by the string pool. */
void str_pool_destroy(struct str_pool *);

/* Returns string pool reference for the string.
 * Note: String should not be null-terminated. */
struct str_ref str_pool_xget_ref(struct str_pool *,
				 const char *chars,
				 size_t len);

/* Get chars of the string. Note: returned str will not be null-terminated. */
void str_ref_get_chars(const struct str_ref,
		       const char **out_chars,
		       size_t *out_len);

/* Destroys string reference and decrements its reference count. */
void str_ref_destroy(struct str_ref);

/* Clone a string reference. (increment its reference count.) */
struct str_ref str_ref_clone(struct str_ref r);


#endif
