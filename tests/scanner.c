#include "../include/common.h"
#include "../include/grammar.h"
#include "../include/scanner.h"
#include "../include/string_pool.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


void test_input(struct scanner *s,
		const char *input,
		const enum tk_id *ids,
		const union seminfo_data *seminfos,
		size_t len)
{
	scanner_feed(s, input);

	enum tk_id tk_id;
	struct seminfo seminfo;

	for (size_t i = 0; i < len; i++) {
		scanner_xnext(s, &tk_id, &seminfo);

		Lw_assert(tk_id == ids[i],
			  "token type missmatch");

		if (tk_id == TK_NUMBER)
			Lw_assert(seminfo.seminfo.number == seminfos[i].number,
				  "number missmatch");

		if (tk_id == TK_NUMBER)
			Lw_assert(seminfo.seminfo.number == seminfos[i].number,
				  "number missmatch");

		if (tk_id == TK_STR || tk_id == TK_IDENT) {
			Lw_assert(seminfo.seminfo.str_ref.id == seminfos[i].str_ref.id,
				  "str_ref.id missmatch (str)");
		}
	}

	scanner_xnext(s, &tk_id, &seminfo);
	Lw_assert(tk_id == TK_EOF || tk_id == TK_INVALID, "still has tokens");
}


int main(void)
{
	scanner_xstatic_init();

	struct str_pool strings;
	str_pool_xinit(&strings);

	struct scanner s;
	scanner_xinit(&s, &strings);

	test_input(&s,
		   "    test  123 test2 < =  \n test   ; 321.123 >= >",
		   (enum tk_id[]) {
			TK_IDENT, TK_INTEGER, TK_IDENT, TK_LT, TK_EQ, TK_IDENT,
			TK_SEMI, TK_NUMBER, TK_GT_EQ, TK_GT, TK_EOF
		   },
		   (union seminfo_data[]) {
			[0] = { .str_ref.id = 0 },
			[1] = { .integer = 123 },
			[2] = { .str_ref.id = 1 },
			[5] = { .str_ref.id = 0 },
			[7] = { .number = 321.123 },
		   },
		   11);

	test_input(&s,
		   "    valid if ınvalıd ",
		   (enum tk_id[]) {
			TK_IDENT, TK_IF, TK_INVALID
		   },
		   (union seminfo_data[]) {
			[0] = { .str_ref.id = 2 },
		   },
		   3);

	test_input(&s,
		   " \"string\" \"\\\"\" \"\" \"\\\\\" \"\\\\\\\"\" \"string\"",
		   (enum tk_id[]) {
			TK_STR, TK_STR, TK_STR, TK_STR, TK_STR, TK_STR
		   },
		   (union seminfo_data[]) {
			{ .str_ref.id = 3 },
			{ .str_ref.id = 4 },
			{ .str_ref.id = UINT32_MAX },
			{ .str_ref.id = 5 },
			{ .str_ref.id = 6 },
			{ .str_ref.id = 3 },
		   },
		   6);

	test_input(&s,
		   "\"unterminated string ",
		   (enum tk_id[]) {
			TK_INVALID
		   },
		   NULL,
		   1);

	scanner_new_line(&s);

	test_input(&s,
		   "\"invalid escape sequence \\a ",
		   (enum tk_id[]) {
			TK_INVALID
		   },
		   NULL,
		   1);

	scanner_new_line(&s);

	str_pool_destroy(&strings);

	scanner_static_destroy();
}
