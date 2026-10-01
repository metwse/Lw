/* Abstract representation of Lw programs. */

#ifndef AST_H
#define AST_H

#include "string_pool.h"
#include "value.h"

#include "../vendor/rdesc/include/rdesc.h"

#include <stdio.h>


/* Top-level item (itemaration or stmt) */
struct ast_item {
	enum ast_item_kind {
		AST_ITEM_STRUCT,
		AST_ITEM_ENUM,
		AST_ITEM_TRAIT,
		AST_ITEM_IMPL,
		AST_ITEM_FN,
		AST_ITEM_LET,
		AST_ITEM_STMT,
	} kind;

	struct ast_item_data {
		struct ast_struct *struct_item;
		struct ast_enum *enum_item;
		struct ast_trait *trait;
		struct ast_impl *impl;
		struct ast_fn *fn;
		struct ast_let *let;
		struct ast_stmt *stmt;
	} data;

	int line;
};

struct ast_struct {
	struct str_ref name;

	enum ast_struct_kind {
		AST_STRUCT_STRUCT,
		AST_STRUCT_TUPLE,
		AST_STRUCT_UNIT,
	} kind;

	size_t field_count;
	struct str_ref *struct_field_names;
};

struct ast_enum {
	struct str_ref name;

	size_t variant_count;
	struct ast_struct *variants;
};

struct ast_trait {
	struct str_ref name;

	size_t fn_count;
	struct ast_fn *fns;
};

struct ast_impl {
	struct ast_struct *impl_for;
	struct ast_trait *impl;

	size_t fn_count;
	struct ast_fn *fns;
};

struct ast_fn {
	struct str_ref name;

	size_t param_count;
	struct str_ref *param_names;
};

struct ast_let {
	struct str_ref name;

	struct ast_expr *opt_value;
};

struct ast_stmt {
	enum ast_stmt_kind {
		AST_STMT_NONE,
		AST_STMT_RETURN,
		AST_STMT_EXPR,
	} kind;

	union ast_stmt_data {
		struct ast_expr *return_opt_value;
		struct ast_expr *expr_discard_result;
	} data;
};

struct ast_expr {
	enum ast_expr_kind {
		AST_EXPR_BINARY_OP,
		AST_EXPR_UNARY_OP,
		AST_EXPR_IF,
		AST_EXPR_WHILE,
		AST_EXPR_BLOCK,
		AST_EXPR_CONSTANT,
		AST_EXPR_STR_LITERAL,
		AST_EXPR_VARIABLE,
	} kind;

	union ast_expr_data {
		struct ast_expr_binary_op {
			enum ast_expr_binary_op_kind {
				/* arithmetic */
				AST_EXPR_BINARY_OP_ADD,
				AST_EXPR_BINARY_OP_SUB,
				AST_EXPR_BINARY_OP_MULT,
				AST_EXPR_BINARY_OP_DIV,
				AST_EXPR_BINARY_OP_MOD,

				/* comparison/partial comparison/equality */
				AST_EXPR_BINARY_OP_GT,
				AST_EXPR_BINARY_OP_GT_EQ,
				AST_EXPR_BINARY_OP_LT,
				AST_EXPR_BINARY_OP_LT_EQ,
				AST_EXPR_BINARY_OP_EQ,
				AST_EXPR_BINARY_OP_NOT_EQ,

				/* cannot overload */
				AST_EXPR_BINARY_OP_ASGN,
				AST_EXPR_BINARY_OP_OR,
				AST_EXPR_BINARY_OP_AND,
			} kind;
			struct ast_expr *lhs;
			struct ast_expr *rhs;
		} *binary_op;

		struct ast_expr_unary_op {
			enum ast_expr_unary_op_kind {
				AST_EXPR_UNARY_OP_NEG,
				AST_EXPR_UNARY_OP_NOT,
				AST_EXPR_UNARY_OP_CALL,
				AST_EXPR_UNARY_OP_INDEX,
				AST_EXPR_UNARY_OP_ITEM,
			} kind;
			struct ast_expr_unary_op_data {
				struct ast_expr_unary_op_call {
					size_t arg_count;
					struct ast_expr *args;
				} call;
				/* value to index with */
				struct ast_expr *index;
				struct str_ref item;
			} data;
			struct ast_expr *expr;
		} *unary_op;

		struct ast_expr_if {
			enum ast_expr_if_rest_kind {
				AST_EXPR_IF_NONE,
				AST_EXPR_IF_ELSE,
				AST_EXPR_IF_ELSE_IF,
			} rest_kind;
			union ast_expr_if_rest_data {
				struct ast_expr_block *else_block;
				struct ast_expr_if *else_if;
			} rest_data;
			struct ast_expr *cond;
			struct ast_expr_block *then;
		} *if_expr;

		struct ast_expr_while {
			struct ast_expr *cond;
			struct ast_expr_block *block;
		} *while_expr;

		struct ast_expr_block {
			/* items excluding the return expression */
			size_t item_count;
			struct ast_item **items;

			struct ast_expr *opt_return_value;
		} *block;

		struct val constant;

		struct str_ref str_literal;

		struct str_ref variable;
	} data;
};


/* Convert a rdesc parse tree to Lw AST. */
struct ast_item *ast_new(struct rdesc_node);

/* Format and print the AST root. */
void ast_pretty_print(FILE *, const struct ast_item *);

/* Free resources allocated by the AST. */
void ast_destroy(struct ast_item *);


#endif
