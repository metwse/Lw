#include "../include/config.h"

#include "../include/ast.h"
#include "../include/common.h"
#include "../include/string_pool.h"
#include "../include/value.h"

#include <stdio.h>


#define print(...) do { \
		fprintf(out, __VA_ARGS__); \
	} while (0)
#define print_indented(...) do { \
		print_indent; \
		print(__VA_ARGS__); \
	} while (0)
#define print_indent do { \
		fprintf(out, "%*s", indent * 4, ""); \
	} while (0)

#define define_printer(ty) \
	static void print_ ## ty(FILE *out _unused, \
				 int indent _unused, \
				 const struct ast_ ## ty *ty _unused)
#define define_printer2(ty, name) \
	static void print_ ## ty(FILE *out _unused, \
				 int indent _unused, \
				 const struct ast_ ## ty *name _unused)
#define call_printer(fn, ast_node) print_ ## fn(out, indent, ast_node)
#define cast(ty) const struct ast_ ## ty *ty = (const struct ast_ ## ty *) ast_node;
#define cast2(ty, name) const struct ast_ ## ty *name = (const struct ast_ ## ty *) ast_node;


define_printer2(struct, struct_item);
define_printer2(enum, enum_item);
define_printer(trait);
define_printer(impl);
define_printer(fn);
define_printer(let);
define_printer(expr_block);
define_printer(expr_if);
define_printer(expr_while);
define_printer(expr);
define_printer(stmt);
define_printer(item);


define_printer2(struct, struct_item)
{
}

define_printer2(enum, enum_item)
{
}

define_printer(trait)
{
}

define_printer(impl)
{
}

define_printer(fn)
{
}

define_printer(let)
{
}

define_printer2(expr_block, block)
{
	print("block: {\n");
	indent++;

	for (size_t i = 0; i < block->item_count; i++) {
		print_indented("(%zu) ", i);
		call_printer(item, block->items[i]);
	}

	if (block->opt_return_value != NULL) {
		print_indented("(return) ");
		call_printer(expr, block->opt_return_value);;
	}

	indent--;
	print_indented("}\n");
}


define_printer2(expr_if, if_expr)
{
	print("if: {\n");
	indent++;

	print_indented("(cond) ");
	call_printer(expr, if_expr->cond);
	print_indented("(then) ");
	call_printer(expr_block, if_expr->then);
	switch (if_expr->rest_kind) {
	case AST_EXPR_IF_NONE:
		break;

	case AST_EXPR_IF_ELSE:
		print_indented("(else) ");
		call_printer(expr_block, if_expr->rest_data.else_block);
		break;

	case AST_EXPR_IF_ELSE_IF:
		print_indented("(else) ");
		call_printer(expr_if, if_expr->rest_data.else_if);
		break;
	}

	indent--;
	print_indented("}\n");
}

define_printer2(expr_while, while_expr)
{
	print("while: {\n");
	indent++;

	print_indented("(cond) ");
	call_printer(expr, while_expr->cond);
	print_indented("(do) ");
	call_printer(expr_block, while_expr->block);

	indent--;
	print_indented("}\n");
}

define_printer(expr)
{
	print("expr: {\n");
	indent++;

	const char *binary_op_name_map[] = {
		[AST_EXPR_BINARY_OP_ADD] = "+",
		[AST_EXPR_BINARY_OP_SUB] = "-",
		[AST_EXPR_BINARY_OP_MULT] = "*",
		[AST_EXPR_BINARY_OP_DIV] = "/",
		[AST_EXPR_BINARY_OP_MOD] = "%",

		[AST_EXPR_BINARY_OP_GT] = ">",
		[AST_EXPR_BINARY_OP_GT_EQ] = ">=",
		[AST_EXPR_BINARY_OP_LT] = "<",
		[AST_EXPR_BINARY_OP_LT_EQ] = "<=",
		[AST_EXPR_BINARY_OP_EQ] = "==",
		[AST_EXPR_BINARY_OP_NOT_EQ] = "!=",

		[AST_EXPR_BINARY_OP_ASGN] = "=",
		[AST_EXPR_BINARY_OP_OR] = "||",
		[AST_EXPR_BINARY_OP_AND] = "&&",
	};

	const char *unary_op_name_map[] = {
		[AST_EXPR_UNARY_OP_NEG] = "-",
		[AST_EXPR_UNARY_OP_NOT] = "!",
		[AST_EXPR_UNARY_OP_CALL] = "call",
		[AST_EXPR_UNARY_OP_INDEX] = "index",
		[AST_EXPR_UNARY_OP_ITEM] = "item",
	};

	switch (expr->kind) {
	case AST_EXPR_BINARY_OP:
		print_indented("binary op (%s)\n",
			       binary_op_name_map[expr->data.binary_op->kind]);
		print_indented("(lhs) ");
		call_printer(expr, expr->data.binary_op->lhs);
		print_indented("(rhs) ");
		call_printer(expr, expr->data.binary_op->rhs);
		break;

	case AST_EXPR_UNARY_OP:
		print_indented("unary op (%s)\n",
			       unary_op_name_map[expr->data.unary_op->kind]);
		print_indented("(expr) ");
		call_printer(expr, expr->data.unary_op->expr);
		break;

	case AST_EXPR_BLOCK:
		print_indent;
		call_printer(expr_block, expr->data.block);
		break;

	case AST_EXPR_IF:
		print_indent;
		call_printer(expr_if, expr->data.if_expr);
		break;

	case AST_EXPR_WHILE:
		print_indent;
		call_printer(expr_while, expr->data.while_expr);
		break;

	case AST_EXPR_CONSTANT:
		switch (expr->data.constant.type) {
		case VAL_NUMBER:
			print_indented("(number) %"Lw_number_fmt"\n",
				       expr->data.constant.val.number);
			break;

		case VAL_INTEGER:
			print_indented("(integer) %"Lw_integer_fmt"\n",
				       expr->data.constant.val.integer);
			break;

		case VAL_BOOL:
			print_indented("(boolean) %s\n",
				       expr->data.constant.val.boolean ?
						"true" : "false");
			break;

		_unreachable_default;
		}
		break;

	case AST_EXPR_STR_LITERAL: {
		const char *chars;
		size_t len;

		str_ref_get_chars(expr->data.str_literal, &chars, &len);
		print_indented("(string literal) %.*s\n", (int) len, chars);
		break;
	}

	case AST_EXPR_VARIABLE: {
		const char *chars;
		size_t len;

		str_ref_get_chars(expr->data.str_literal, &chars, &len);
		print_indented("(name) %.*s\n", (int) len, chars);
		break;
	}
	}

	indent--;
	print_indented("}\n");
}

define_printer(stmt)
{
	print("stmt: {\n");
	indent++;

	switch (stmt->kind) {
	case AST_STMT_NONE:
		print_indented("noop\n");
		break;

	case AST_STMT_RETURN:
		print_indented("return ");
		if (stmt->data.return_opt_value != NULL)
			call_printer(expr, stmt->data.return_opt_value);
		else
			print("()\n");
		break;

	case AST_STMT_EXPR:
		print_indented("(discarded result) ");
		call_printer(expr, stmt->data.expr_discard_result);
		break;
	}

	indent--;
	print_indented("}\n");
}

define_printer(item)
{
	print("ast_item:%d {\n", item->line);
	indent++;

	switch (item->kind) {
	case AST_ITEM_STRUCT:
		call_printer(struct, item->data.struct_item);
		break;
	case AST_ITEM_ENUM:
		call_printer(enum, item->data.enum_item);
		break;
	case AST_ITEM_TRAIT:
		call_printer(trait, item->data.trait);
		break;
	case AST_ITEM_IMPL:
		call_printer(impl, item->data.impl);
		break;
	case AST_ITEM_FN:
		call_printer(fn, item->data.fn);
		break;
	case AST_ITEM_LET:
		print_indent;
		call_printer(let, item->data.let);
		break;
	case AST_ITEM_STMT:
		print_indent;
		call_printer(stmt, item->data.stmt);
		break;
	}

	indent--;
	print_indented("}\n");
}

void ast_pretty_print(FILE *out, const struct ast_item *item)
{
	print_item(out, 0, item);
}
