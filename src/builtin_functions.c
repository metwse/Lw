#include "../include/config.h"

#include "../include/builtin_functions.h"
#include "../include/common.h"
#include "../include/gc.h"
#include "../include/string_pool.h"
#include "../include/value.h"
#include "../include/vm.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>


static struct val hello_world(struct vm *vm _unused,
			      struct val argv[] _unused,
			      uint32_t argc _unused)
{
	printf("Hello, world! Got %"PRIu32" arguments.\n", argc);

	return UNIT_VAL;
}

static struct val gc_run(struct vm *vm,
			 struct val argv[] _unused,
			 uint32_t argc _unused)
{
	struct gc_result res = vm_gc_mark(vm);

	printf("== GC run ==\n"
	       "%zu of %zu objects\n",
	       res.objs, fstack_objects_len(&vm->objects));

	vm_gc_sweep(vm);
	return UNIT_VAL;
}

static struct val str_pool_info(struct vm *vm,
				struct val argv[] _unused,
				uint32_t argc _unused)
{
	printf("== Str Pool Sizes ==\n");

#define print_strings(m) do { \
	struct fhmap_str_pool_strings_entry e; \
	struct fhmap_str_pool_strings_it it = \
		fhmap_str_pool_strings_iter(&m); \
	while (fhmap_str_pool_strings_iter_next(&it, &e)) { \
		printf("(id %"PRIu32", %zu refs: %.*s) ", \
			e.value->id, e.value->ref_count, (int) e.key_len, e.key); \
	} \
} while (0)

	printf("Short strings: %zu\n",
	       fhmap_str_pool_strings_used(&vm->strings->maps[0].strings));
	print_strings(vm->strings->maps[0].strings);
	printf("\nStrings: %zu\n",
	       fhmap_str_pool_strings_used(&vm->strings->maps[1].strings));
	print_strings(vm->strings->maps[1].strings);
	printf("\nLong strings: %zu\n",
	       fhmap_str_pool_strings_used(&vm->strings->maps[2].strings));
	print_strings(vm->strings->maps[2].strings);
	putc('\n', stdout);

	return UNIT_VAL;
}

static struct val print(struct vm *vm _unused,
			struct val argv[],
			uint32_t argc)
{
	for (uint32_t i = 0; i < argc; i++) {
		struct val v = argv[i];

		switch (v.type) {
		case VAL_NUMBER:
			printf("%"Lw_number_fmt, AS_NUMBER(v));
			break;

		case VAL_INTEGER:
			printf("%"Lw_integer_fmt, AS_INTEGER(v));
			break;

		case VAL_BOOL:
			printf(AS_BOOL(v) ? "true" : "false");
			break;

		case VAL_UNIT:
			printf("()");
			break;

		case VAL_OBJ:
			obj_print(AS_OBJ(v), vm);
			break;
		}

		if (i != argc - 1)
			putc(' ', stdout);
	}

	return UNIT_VAL;
}

static struct val println(struct vm *vm,
			  struct val argv[],
			  uint32_t argc)
{
	print(vm, argv, argc);
	putc('\n', stdout);

	return UNIT_VAL;
}

static struct val typeof(struct vm *vm,
			 struct val argv[],
			 uint32_t argc)
{
	Lw_assert(argc == 1, "expected one argument");

	struct str_ref str_ref = str_pool_xget_ref(vm->strings,
						   val_names[argv[0].type],
						   strlen(val_names[argv[0].type]));

	return OBJ_VAL((struct obj *) obj_str_literal_new(vm, str_ref));
}

struct builtin_function builtin_functions[] = {
	{ "hello_world", hello_world },
	{ "gc_run", gc_run },
	{ "str_pool_info", str_pool_info },
	{ "print", print },
	{ "println", println },
	{ "typeof", typeof },
};

size_t builtin_functions_len = 6;
