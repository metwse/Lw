#include "../include/config.h"

#include "../include/ast.h"
#include "../include/common.h"
#include "../include/string_pool.h"
#include "../include/value.h"

#include <stdio.h>


#define print(...) do { \
		fprintf(out, "%*s", indent * 4, ""); \
		print1(__VA_ARGS__); \
	} while (0)
#define print1(...) do { \
		fprintf(out, __VA_ARGS__); \
	} while (0)

#define define_printer(name) \
	static void print_ ## name(FILE *out _unused, \
				   int indent _unused, \
				   const void *ast_node _unused)
#define call_printer(fn, ast_node) print_ ## fn(out, indent, ast_node)
#define cast(ty) const struct ast_ ## ty *ty = (const struct ast_ ## ty *) ast_node;
#define cast2(ty, name) const struct ast_ ## ty *name = (const struct ast_ ## ty *) ast_node;


define_printer(struct);
define_printer(enum);
define_printer(trait);
define_printer(impl);
define_printer(fn);
define_printer(let);
define_printer(expr_if);
define_printer(expr_while);
define_printer(expr);
define_printer(block_expr);
define_printer(stmt);
define_printer(item);


define_printer(struct)
{
}

define_printer(enum)
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

define_printer(expr_if)
{
	cast2(expr_if, if_expr);
	print("if: {\n");
	indent++;

	print("(cond) ");
	call_printer(expr, if_expr->cond);
	print("(then) ");
	call_printer(block_expr, if_expr->then);
	switch (if_expr->rest_kind) {
	case AST_EXPR_IF_NONE:
		break;

	case AST_EXPR_IF_ELSE:
		print("(else) ");
		call_printer(block_expr, if_expr->rest_data.else_block);
		break;

	case AST_EXPR_IF_ELSE_IF:
		print("(else if)\n");
		call_printer(expr_if, if_expr->rest_data.else_if);
		break;
	}

	indent--;
}

define_printer(expr_while)
{
	cast2(expr_while, while_expr);
	print("while: {\n");
	indent++;

	print("(cond) ");
	call_printer(expr, while_expr->cond);
	print("(do) ");
	call_printer(block_expr, while_expr->block);

	indent--;
}

define_printer(expr)
{
	cast(expr);
	print1("expr: {\n");
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
		print("binary op (%s)\n",
		      binary_op_name_map[expr->data.binary_op->kind]);
		print("(lhs) ");
		call_printer(expr, expr->data.binary_op->lhs);
		print("(rhs) ");
		call_printer(expr, expr->data.binary_op->rhs);
		break;

	case AST_EXPR_UNARY_OP:
		print("unary op (%s)\n",
		      unary_op_name_map[expr->data.unary_op->kind]);
		print(" ");
		call_printer(expr, expr->data.unary_op->expr);
		break;

	case AST_EXPR_IF:
		call_printer(expr_if, expr->data.if_expr);
		break;

	case AST_EXPR_WHILE:
		call_printer(expr_while, expr->data.while_expr);
		break;

	case AST_EXPR_CONSTANT:
		switch (expr->data.constant.type) {
		case VAL_NUMBER:
			print("(number) %"Lw_number_fmt"\n",
					expr->data.constant.val.number);
			break;

		case VAL_INTEGER:
			print("(integer) %"Lw_integer_fmt"\n",
					expr->data.constant.val.integer);
			break;

		case VAL_BOOL:
			print("(boolean) %s\n", expr->data.constant.val.boolean ?
					"true" : "false");
			break;

		_unreachable_default;
		}
		break;

	case AST_EXPR_STR_LITERAL: {
		const char *chars;
		size_t len;

		str_ref_get_chars(expr->data.str_literal, &chars, &len);
		print("(string literal) %*s\n", (int) len, chars);
		break;
	}

	case AST_EXPR_VARIABLE: {
		const char *chars;
		size_t len;

		str_ref_get_chars(expr->data.str_literal, &chars, &len);
		print("(name) %*s\n", (int) len, chars);
		break;
	}
	}

	indent--;
	print("}\n");
}

define_printer(block_expr)
{
	cast(block_expr);
	print1("block_expr: {\n");
	indent++;

	for (size_t i = 0; i < block_expr->item_count; i++) {
		print("(%zu) ", i);
		call_printer(item, block_expr->items[i]);
	}

	if (block_expr->opt_return_value != NULL) {
		print("(return) ");
		call_printer(expr, block_expr->opt_return_value);;
	}

	indent--;
	print("}\n");
}

define_printer(stmt)
{
	cast(stmt);
	print("stmt: {\n");
	indent++;

	switch (stmt->kind) {
	case AST_STMT_NONE:
		print("noop\n");
		break;

	case AST_STMT_RETURN:
		print("return ");
		if (stmt->data.opt_return_value != NULL) {
			call_printer(expr, stmt->data.opt_return_value);
		} else {
			print("()\n");
		}
		break;

	case AST_STMT_EXPR:
		print("(discarded result) ");
		call_printer(expr, stmt->data.expr_discard_result);
		break;

	case AST_STMT_BLOCK:
		call_printer(block_expr, stmt->data.block);
		break;
	}

	indent--;
	print("}\n");
}

define_printer(item)
{
	cast(item);
	print1("ast_item:%d {\n", item->line);
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
		call_printer(let, item->data.let);
		break;
	case AST_ITEM_STMT:
		call_printer(stmt, item->data.stmt);
		break;
	}

	indent--;
	print("}\n");
}

void ast_pretty_print(FILE *out, const struct ast_item *item)
{
	print_item(out, 0, item);
}
