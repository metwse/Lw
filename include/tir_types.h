/* Definitions for Lw type intermediate representation. */

#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>


struct tir_type_pool;

/* Type reference counter. */
struct tir_type_ref {
	struct tir_type_pool *p;
	size_t id;
};

/* Trait reference counter. */
struct tir_trait_ref {
	struct tir_type_pool *p;
	size_t id;
};

/* Common fields for all types. */
struct tir_type {
	enum tir_type_kind {
		TIR_TYPE_FUNCTION,
		TIR_TYPE_PRODUCT,
		TIR_TYPE_SUM,
		TIR_TYPE_UNIT,
	} kind;

	size_t generic_param_count;
	struct tir_type_ref *generic_params;
};

/* Dynamic type with trait boundaries. */
struct tir_trait {
	size_t generic_param_count;
	struct tir_type_ref *generic_params;

	size_t bound_count;
	struct tir_trait_ref *bounds;
};

/* A special type that takes a number of arguments and returns one. */
struct tir_type_function {
	struct tir_type type;

	size_t arity;
	struct tir_type_ref *arg_types;

	struct tir_type_ref return_type;
};

/* Collection or types. */
struct tir_type_product {
	struct tir_type type;

	size_t field_count;
	struct tir_type_ref *field_types;
};

/* A type that allow multiple variants. */
struct tir_type_sum {
	struct tir_type type;

	size_t variant_count;
	struct tir_type_struct *variants;
};


#endif
