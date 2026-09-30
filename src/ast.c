#include "../include/ast.h"
#include "../include/common.h"
#include "../include/grammar.h"
#include "../include/value.h"

#include "../vendor/rdesc/include/cst_macros.h"
#include "../vendor/rdesc/include/rdesc.h"
#include "../vendor/rdesc/include/util.h"


#define SEMINFO(n) (*(struct seminfo *) rseminfo(n))
#define SEMINFO_STR_REF(n) (*(struct seminfo *) rseminfo(n)).seminfo.str_ref
#define SEMINFO_NUMBER(n) (*(struct seminfo *) rseminfo(n)).seminfo.number
#define SEMINFO_INTEGER(n) (*(struct seminfo *) rseminfo(n)).seminfo.integer


static struct ast_expr *parse_expr(struct rdesc_node);
static struct ast_expr_if *parse_expr_if(struct rdesc_node);
void parse_expr_recursive(struct rdesc_node, struct ast_expr *);
static struct ast_expr_while *parse_expr_while(struct rdesc_node);
static struct ast_block_expr *parse_expr_block(struct rdesc_node);

static struct ast_item *parse_item(struct rdesc_node);
static struct ast_let *parse_let(struct rdesc_node);
static struct ast_stmt *parse_stmt(struct rdesc_node);


static struct ast_block_expr *parse_expr_block(struct rdesc_node n _unused)
{
	struct ast_block_expr *block_expr = xmalloc(sizeof(struct ast_block_expr));
	size_t item_count = 0;

#define traverse_block_expr(item, expr) do { \
	struct rdesc_node cur = rchild(n, 1); \
	struct rdesc_node item_node _unused; \
	struct rdesc_node expr_node _unused; \
	while (true) { \
		bool break_while = false; \
		switch (ralt_idx(cur)) { \
		case 0: \
			if (ralt_idx(rchild(cur, 1)) == 1) { \
				expr_node = rchild(cur, 0); \
				expr; \
				break_while = true; \
			} else { \
				item_node = rchild(cur, 0); \
				item; \
				cur = rchild(rchild(cur, 1), 1); \
			} \
			break; \
		case 1: \
			item_node = rchild(cur, 0); \
			item; \
			cur = rchild(cur, 1); \
			break; \
		case 2: \
			break_while = true; \
			break; \
		} \
		if (break_while) \
			break; \
	} \
} while (0)

	traverse_block_expr(item_count++, ((void) 0));

	block_expr->items = xmalloc(sizeof(struct ast_item *) * item_count);
	block_expr->item_count = item_count;
	block_expr->opt_return_value = NULL;

	size_t i = 0;
	traverse_block_expr(block_expr->items[i++] = parse_item(item_node),
			    block_expr->opt_return_value = parse_expr(expr_node));

	return NULL;
}

/* Parse NT_EXPR, NT_EXPR_WITHOUT_BLOCK, or NT_EXPR_WITH_BLOCK as ast_exr. */
static struct ast_expr *parse_expr(struct rdesc_node n)
{
	struct ast_expr *expr = xmalloc(sizeof(struct ast_expr));

	if (rid(n) == NT_EXPR)
		n = rchild(n, 0);

	switch (rid(n)) {
	case NT_EXPR_WITHOUT_BLOCK:
		parse_expr_recursive(rchild(n, 0), expr);
		break;

	case NT_EXPR_WITH_BLOCK:
		switch (rid(n)) {
		case NT_EXPR_IF: {
			expr->kind = AST_EXPR_IF;
			expr->data.if_expr = parse_expr_if(rchild(n, 0));
			break;
		}

		case NT_EXPR_WHILE:
			expr->kind = AST_EXPR_WHILE;
			expr->data.while_expr = parse_expr_while(rchild(n, 0));
			break;

		_unreachable_default;  // GCOVR_EXCL_LINE
		}

		break;

	_unreachable_default;  // GCOVR_EXCL_LINE
	}

	return expr;
}


static enum ast_expr_binary_op_kind binary_op_map[][4] = {
	[NT_EXPR_LOGIC_OR] = { AST_EXPR_BINARY_OP_OR, },
	[NT_EXPR_LOGIC_AND] = { AST_EXPR_BINARY_OP_AND, },
	[NT_EXPR_EQUALITY] = { AST_EXPR_BINARY_OP_NOT_EQ,
			       AST_EXPR_BINARY_OP_EQ, },
	[NT_EXPR_COMPARISON] = { AST_EXPR_BINARY_OP_GT,
				 AST_EXPR_BINARY_OP_GT_EQ,
				 AST_EXPR_BINARY_OP_LT,
				 AST_EXPR_BINARY_OP_LT_EQ, },
	[NT_EXPR_TERM] = { AST_EXPR_BINARY_OP_SUB,
			   AST_EXPR_BINARY_OP_ADD, },
	[NT_EXPR_FACTOR] = { AST_EXPR_BINARY_OP_DIV,
			     AST_EXPR_BINARY_OP_MULT,
			     AST_EXPR_BINARY_OP_MOD, },
};

static enum ast_expr_unary_op_kind unary_op_map[][3] = {
	[NT_EXPR_UNARY_PREFIX] = { 0,
				   AST_EXPR_UNARY_OP_NEG,
				   AST_EXPR_UNARY_OP_NOT, },
	[NT_EXPR_UNARY_POSTFIX] = { AST_EXPR_UNARY_OP_CALL,
				    AST_EXPR_UNARY_OP_INDEX,
				    AST_EXPR_UNARY_OP_ITEM, },
};

void parse_expr_recursive(struct rdesc_node n, struct ast_expr *expr)
{
	switch (rid(n)) {
	/* the right-associative assignment binary operator */
	case NT_EXPR_ASGN: {
		struct rdesc_node rhs = rchild(n, 1);
		if (ralt_idx(rhs) == 0) {
			expr->kind = AST_EXPR_BINARY_OP;
			rdesc_flip_left(n, 0);  /* flip <expr_logic_or> */

			expr->data.binary_op =
				xmalloc(sizeof(struct ast_expr_binary_op));

			expr->data.binary_op->kind = AST_EXPR_BINARY_OP_ASGN;
			expr->data.binary_op->lhs = parse_expr(rchild(n, 0));
			expr->data.binary_op->rhs = parse_expr(rchild(rhs, 1));
		} else {
			rdesc_flip_left(n, 0);  /* flip <expr_logic_or> */
			parse_expr_recursive(rchild(n, 0), expr);
		}
		break;
	}

	/* left-associative binary operators */
	case NT_EXPR_LOGIC_OR:
	case NT_EXPR_LOGIC_AND:
	case NT_EXPR_EQUALITY:
	case NT_EXPR_COMPARISON:
	case NT_EXPR_TERM:
	case NT_EXPR_FACTOR:
		if (ralt_idx(n) == 0) {
			expr->kind = AST_EXPR_BINARY_OP;
			if (rid(n) != NT_EXPR_FACTOR)  /* factor has unary op children */
				rdesc_flip_left(n, 0);

			expr->data.binary_op =
				xmalloc(sizeof(struct ast_expr_binary_op));

			expr->data.binary_op->kind =
				binary_op_map[rid(n)][ralt_idx(rchild(n, 1))];
						       /* ^ the operator */
			expr->data.binary_op->lhs = parse_expr(rchild(n, 0));
			expr->data.binary_op->rhs = parse_expr(rchild(n, 2));
		} else {
			parse_expr_recursive(rchild(n, 0), expr);
		}
		break;

	case NT_EXPR_UNARY_PREFIX:
		if (ralt_idx(n) == 0 && ralt_idx(rchild(n, 0)) != 0) {
			expr->kind = AST_EXPR_UNARY_OP;

			expr->data.unary_op =
				xmalloc(sizeof(struct ast_expr_unary_op));

			expr->data.unary_op->kind =
				unary_op_map[rid(n)][ralt_idx(rchild(n, 0))];
			expr->data.unary_op->expr = parse_expr(rchild(n, 1));
		} else {
			parse_expr_recursive(rchild(n, 0), expr);
		}
		break;

	case NT_EXPR_UNARY_POSTFIX: {
		int op = ralt_idx(rchild(n, 1));
		if (op != 4) {
			expr->kind = AST_EXPR_UNARY_OP;

			expr->data.unary_op =
				xmalloc(sizeof(struct ast_expr_unary_op));

			expr->data.unary_op->kind =
				unary_op_map[op][ralt_idx(rchild(n, 0))];
			switch (op) {
			case 0:
				Lw_fatal("call operator is not implemented yet");
				break;

			case 1:
				Lw_fatal("index operator is not implemented yet");
				break;

			case 2:
				Lw_fatal("item operator is not implemented yet");
				break;
			}
		} else {
			parse_expr_recursive(rchild(n, 0), expr);
		}
		break;
	}

	case NT_EXPR_PRIMARY: {
		switch (ralt_idx(n)) {
		case 0:
			expr->kind = AST_EXPR_CONSTANT;
			expr->data.constant = NUMBER_VAL(SEMINFO_NUMBER(n));
			break;
		case 1:
			expr->kind = AST_EXPR_CONSTANT;
			expr->data.constant = INTEGER_VAL(SEMINFO_INTEGER(n));
			break;
		case 2:
			expr->kind = AST_EXPR_STR_LITERAL;
			expr->data.str_literal = SEMINFO_STR_REF(n);
			break;
		case 3:
			expr->kind = AST_EXPR_CONSTANT;
			expr->data.constant = BOOL_VAL(true);
			break;
		case 4:
			expr->kind = AST_EXPR_CONSTANT;
			expr->data.constant = BOOL_VAL(false);
			break;
		case 5:
			parse_expr_recursive(rchild(n, 1), expr);
			break;
		case 6:
			expr->kind = AST_EXPR_VARIABLE;
			expr->data.variable = SEMINFO_STR_REF(n);
			break;
		}
		break;
	}
	}
}

static struct ast_expr_if *parse_expr_if(struct rdesc_node n)
{
	struct ast_expr_if *expr_if = xmalloc(sizeof(struct ast_expr_if));

	struct rdesc_node optelse = rchild(n, 3);
	switch (ralt_idx(optelse)) {
	case 0:
		expr_if->rest_kind = AST_EXPR_IF_ELSE_IF;
		expr_if->rest_data.else_if =
			parse_expr_if(rchild(optelse, 1));
		break;

	case 1:
		expr_if->rest_kind = AST_EXPR_IF_ELSE;
		expr_if->rest_data.else_block =
			parse_expr_block(rchild(optelse, 1));
		break;

	case 2:
		expr_if->rest_kind = AST_EXPR_IF_NONE;
		break;
	}

	expr_if->cond = parse_expr(rchild(n, 1));
	expr_if->then = parse_expr_block(rchild(n, 2));

	return expr_if;
}

static struct ast_expr_while *parse_expr_while(struct rdesc_node n)
{
	struct ast_expr_while *expr_while = xmalloc(sizeof(struct ast_expr_while));

	expr_while->cond = parse_expr(rchild(n, 1));
	expr_while->block = parse_expr_block(rchild(n, 2));

	return expr_while;
}

static struct ast_let *parse_let(struct rdesc_node n)
{
	struct ast_let *let = xmalloc(sizeof(struct ast_let));

	let->name = SEMINFO_STR_REF(rchild(n, 1));

	struct rdesc_node let_def = rchild(n, 2);

	if (ralt_idx(let_def) == 0)
		let->opt_value = parse_expr(rchild(n, 1));
	else
		let->opt_value = NULL;

	return let;
}

/* parse NT_STMT or NT_EXPR as ast_stmt. */
static struct ast_stmt *parse_stmt(struct rdesc_node n)
{
	struct ast_stmt *stmt = xmalloc(sizeof(struct ast_stmt));

	if (rid(n) == NT_EXPR) {
		stmt->kind = AST_STMT_EXPR;
		stmt->data.expr_discard_result = parse_expr(n);
		return stmt;
	}

	switch (ralt_idx(n)) {
	case 0:
		stmt->kind = AST_STMT_NONE;
		break;

	case 1:
		stmt->kind = AST_STMT_EXPR;
		stmt->data.expr_discard_result =
			parse_expr(rchild(rchild(n, 0), 0));
		break;

	case 2:
		stmt->kind = AST_STMT_RETURN;
		if (ralt_idx(rchild(rchild(n, 0), 1)) == 0) {
			stmt->data.opt_return_value =
				parse_expr(rchild(rchild(n, 0), 0));
		} else {
			stmt->data.opt_return_value = NULL;
		}
		break;

	case 3:
		stmt->kind = AST_STMT_BLOCK;
		stmt->data.block = parse_expr_block(rchild(n, 0));
		break;
	}

	return stmt;
}

/* parse NT_ITEM or NT_EXPR as ast_item */
struct ast_item *parse_item(struct rdesc_node n)
{
	struct ast_item *item = xmalloc(sizeof(struct ast_item));

	if (rid(n) == NT_EXPR) {
		item->kind = AST_ITEM_STMT;
		item->data.stmt = parse_stmt(n);
		/* TODO: update line in stmt */
		return item;
	}

#define update_line(n) item->line = SEMINFO(n).line;

	n = rchild(n, 0);
	switch (rid(n)) {
	case NT_ITEM_STRUCT:
		Lw_fatal("structs are not implemented yet");
		break;

	case NT_ITEM_ENUM:
		Lw_fatal("enums are not implemented yet");
		break;

	case NT_ITEM_TRAIT:
		Lw_fatal("traits are not implemented yet");
		break;

	case NT_ITEM_IMPL:
		Lw_fatal("struct/trait impls are not implemented yet");
		break;

	case NT_ITEM_FN:
		Lw_fatal("functions are not implemented yet");
		break;

	case NT_ITEM_LET:
		item->kind = AST_ITEM_LET;
		item->data.let = parse_let(n);
		update_line(rchild(n, 0));
		break;

	case NT_STMT:
		item->kind = AST_ITEM_STMT;
		item->data.stmt = parse_stmt(n);
		/* TODO: update line in stmt */
		break;

	_unreachable_default;  // GCOVR_EXCL_LINE
	}

	return item;
}

struct ast_item *ast_new(struct rdesc_node n)
{
	return parse_item(n);
}

/* void ast_destroy(struct ast_item *); */
