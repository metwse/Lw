#include "../include/common.h"
#include "../include/string_pool.h"

#include <stdint.h>


void str_pool_xinit(struct str_pool *p)
{
	p->last_id = 0;
	for (int i = 0; i < 3; i++) {
		fhmap_str_pool_strings_xinit(&p->maps[i].strings);
		fhmap_str_pool_rev_map_xinit(&p->maps[i].rev);
	}
}

void str_pool_destroy(struct str_pool *p)
{
	for (int i = 0; i < 3; i++) {
		fhmap_str_pool_strings_destroy(&p->maps[i].strings);
		fhmap_str_pool_rev_map_destroy(&p->maps[i].rev);
	}
}

struct str_ref str_pool_xget_ref(struct str_pool *p,
				 const char *chars,
				 size_t len)
{
	if (len == 0)
		return (struct str_ref) { .p = NULL, .id = UINT32_MAX, };

	struct fhmap_str_pool_strings *m_strings = &p->maps[0].strings;
	struct fhmap_str_pool_rev_map *m_rev = &p->maps[0].rev;
	if (8 < len && len < 20) {
		m_strings = &p->maps[1].strings;
		m_rev = &p->maps[1].rev;
	} else if (20 <= len) {
		m_strings = &p->maps[2].strings;
		m_rev = &p->maps[2].rev;
	}

	struct str_pool_string *string =
		fhmap_str_pool_strings_get_mut(m_strings, chars, len);

	if (string == NULL) {
		size_t new_id = p->last_id++;
		struct str_pool_string new_string = {
			.id = new_id,
			.ref_count = 1,
		};

		struct fhmap_str_pool_strings_entry_mut e;
		fhmap_str_pool_strings_xinserte(m_strings, chars, len, &new_string, &e);
		fhmap_str_pool_rev_map_xinsert2(m_rev, &new_id, &e);

		return (struct str_ref) { .p = p, .id = new_id };
	} else {
		string->ref_count++;

		return (struct str_ref) { .p = p, .id = string->id };
	}
}

#define get_entry \
	const struct fhmap_str_pool_strings_entry_mut *e = NULL; \
	int map_index; \
	for (map_index = 0; map_index < 3; map_index++) { \
		e = fhmap_str_pool_rev_map_get2(&r.p->maps[map_index].rev, &r.id); \
		if (e != NULL) \
			break; \
	} \
	Lw_assert(e != NULL, "string got destroyed (reference count was zero)")

void str_ref_get_chars(const struct str_ref r,
		       const char **out_chars,
		       size_t *out_len)
{
	if (r.id == UINT32_MAX) {
		*out_len = 0;
		*out_chars = NULL;
		return;
	}

	get_entry;

	*out_chars = e->key;
	*out_len = e->key_len;
}

void str_ref_destroy(struct str_ref r)
{
	if (r.id == UINT32_MAX)
		return;

	get_entry;

	e->value->ref_count--;

	if (e->value->ref_count == 0) {
		Lw_assert(
		fhmap_str_pool_strings_remove(&r.p->maps[map_index].strings,
					      e->key,
					      e->key_len,
					      NULL)
		, "");
		Lw_assert(
		fhmap_str_pool_rev_map_remove2(&r.p->maps[map_index].rev, &r.id, NULL)
		, "");
	}
}

struct str_ref str_ref_clone(struct str_ref r)
{
	if (r.id == UINT32_MAX)
		return r;

	get_entry;

	e->value->ref_count++;

	return r;
}
