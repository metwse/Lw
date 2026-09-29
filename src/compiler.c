#include "compiler_internal.h"

#include "../include/chunk.h"
#include "../include/common.h"
#include "../include/instructions.h"
#include "../include/grammar.h"
#include "../include/object.h"
#include "../include/string_pool.h"
#include "../include/value.h"

#include "../vendor/rdesc/include/cst_macros.h"
#include "../vendor/rdesc/include/util.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define SEMINFO(n) (((struct seminfo *) rseminfo(n))->seminfo)

#define SEMINFO_NUMBER(n) (SEMINFO(n).number)
#define SEMINFO_INTEGER(n) (SEMINFO(n).integer)
#define SEMINFO_STR_REF(n) (SEMINFO(n).str_ref)


/* shall procide NT_DECL */
struct chunk chunk_xcompile(struct vm *vm, struct rdesc_node n _unused)
{
	struct chunk c;
	struct compiler current;

	chunk_xinit(&c);
	compiler_xinit(&current, NULL, vm);

	chunk_xwrite_inst(&c, current.line, (struct inst) { .op = OP_UNIT });
	chunk_xwrite_inst(&c, current.line, (struct inst) { .op = OP_RETURN });

	compiler_destroy(&current);

	chunk_compact(&c);
	return c;
}
