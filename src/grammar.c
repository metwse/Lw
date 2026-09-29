#include "../include/grammar.h"
#include "../include/string_pool.h"

#include "../vendor/rdesc/include/rule_macros.h"


const char *const tk_names[TK_COUNT] = {
	"{", "}", "(", ")", "[", "]",
	":", ",", ".", "%", "+", ";", "/", "*",

	"!", "!=", "=", "==", ">", ">=", "<", "<=", "-", "->", "|", "||",

	"&&",

	"@IDENT", "@STR", "@NUMBER", "@INTEGER",

	/* generated using :'<,'>s/TK_\(\w*\)/"\L\1"/g */
	"break", "continue", "else", "enum", "false", "fn", "for",
	"if", "impl", "let", "loop", "return", "self", "self_ty",
	"struct", "trait", "true", "type", "where", "while",

	"@eof", "@invalid"
};

/* generated using :'<,'>s/NT_\(\w*\)/"\L\1"/g */
const char *const nt_names[NT_COUNT] = {
	"decl",
	"decl_struct",
	"decl_enum",
	"decl_trait",
	"decl_impl",
	"decl_fn",
	"decl_let",

	"type",
	"generic_params",
	"optgeneric_params",
	"bounds",
	"bounds_rest",
	"optbounds",

	"associated_items",
	"associated_item",
	"optassociated_item_block",

	"stmt",
	"stmt_expr",
	"stmt_return",
	"block",
	"block_stmts",
	"block_stmts_expr_optrest",

	"expr",
	"expr_with_block",
	"expr_without_block",
	"optexpr",

	"expr_if", "expr_if_optelse",
	"expr_while",

	"expr_asgn", "expr_asgn_opteq",
	"expr_logic_or", "expr_logic_or_rest",
	"expr_logic_and", "expr_logic_and_rest",
	"expr_equality", "expr_equality_rest", "expr_equality_op",
	"expr_comparison", "expr_comparison_rest", "expr_comparison_op",
	"expr_term", "expr_term_rest", "expr_term_op",
	"expr_factor", "expr_factor_rest", "expr_factor_op",
	"expr_unary_prefix", "expr_unary_prefix_op",
	"expr_unary_postfix", "expr_unary_postfix_ops",
	"expr_primary",

	"tuple",
	"tuple_items",
	"tuple_items_rest",
	"tuple_optitems",

	"decl_fn_def",
	"decl_fn_params", "decl_fn_params_rest",
	"decl_fn_optparams",

	"decl_let_def",
};

const struct rdesc_grammar_symbol production_rules
	[NT_COUNT][NT_MAX_ALTERNATIVE_COUNT + 1][NT_MAX_ALTERNATIVE_SIZE + 1] = {
/* <decl> ::= */ r(
	NT(DECL_STRUCT)
alt	NT(DECL_ENUM)
alt	NT(DECL_TRAIT)
alt	NT(DECL_IMPL)
alt	NT(DECL_FN)
alt	NT(DECL_LET)
),

/* <decl_struct> ::= */ r(
	TK(STRUCT)
),
/* <decl_enum> ::= */ r(
	TK(ENUM)
),
/* <decl_trait> ::= */ r(
	TK(TRAIT)
),
/* <decl_impl> ::= */ r(
	TK(IMPL)
),
/* <decl_fn> ::= */ r(
	TK(FN), TK(IDENT), TK(LPAREN), NT(DECL_FN_OPTPARAMS), TK(RPAREN), NT(DECL_FN_DEF)
),
/* <decl_let> ::= */ r(
	TK(LET), TK(IDENT), NT(DECL_LET_DEF), TK(SEMI)
),

/* <type> ::= */ r(
	TK(IDENT), NT(OPTGENERIC_PARAMS)
alt	TK(IMPL), TK(IDENT), NT(OPTGENERIC_PARAMS)
),
/* <generic_params> ::= */ r(
	TK(LT), TK(GT)
),
/* <optgeneric_params> ::= */
	ropt(NT(GENERIC_PARAMS)),
/* <bounds> ::= */
	rrr(BOUNDS, (TK(IDENT)), (TK(PLUS), TK(IDENT))),
/* <optbounds> ::= */
	ropt(TK(COLON), NT(BOUNDS)),

/* <associated_items> ::= */
	ropt(NT(ASSOCIATED_ITEM), NT(ASSOCIATED_ITEMS)),
/* <associated_item> ::= */ r(
	NT(DECL_FN)
),
/* <optassociated_item_block> ::= */
	ropt(TK(LBRACE), NT(ASSOCIATED_ITEMS), TK(RBRACE)),

/* <stmt> ::= */ r(
	TK(SEMI)
alt	NT(DECL)
alt	NT(STMT_EXPR)
alt	NT(STMT_RETURN)
alt	NT(BLOCK)
),
/* <stmt_expr> ::= */ r(
	NT(EXPR_WITH_BLOCK)
alt	NT(EXPR_WITHOUT_BLOCK), TK(SEMI)
),
/* <stmt_return> ::= */ r(
	TK(RETURN), NT(OPTEXPR), TK(SEMI)
),
/* <block> ::= */ r(
	TK(LBRACE), NT(BLOCK_STMTS), TK(RBRACE)
),
/* <block_stmts> ::= */ r(
	NT(EXPR), NT(BLOCK_STMTS_EXPR_OPTREST)
alt	NT(STMT), NT(BLOCK_STMTS)
alt	EPSILON
),
/* <block_stmts_expr_optrest> ::= */
	ropt(TK(SEMI), NT(BLOCK_STMTS)),

/* <expr> ::= */ r(
	NT(EXPR_WITH_BLOCK)
alt	NT(EXPR_WITHOUT_BLOCK)
),
/* <expr_with_block> ::= */ r(
	NT(EXPR_IF)
),
/* <expr_without_block> ::= */ r(
	NT(EXPR_ASGN)
),
/* <optexpr> ::= */
	ropt(NT(EXPR)),

/* <expr_if> ::= */ r(
	TK(IF), NT(EXPR), NT(BLOCK), NT(EXPR_IF_OPTELSE)
),
/* <expr_if_optelse> ::= */ r(
	TK(ELSE), NT(EXPR_IF)
alt	TK(ELSE), NT(BLOCK)
alt	EPSILON
),

/* <expr_asgn> ::= */ r(
	NT(EXPR_LOGIC_OR), NT(EXPR_ASGN_OPTEQ)
),
/* <expr_asgn_opteq> ::= */
	ropt(TK(EQ), NT(EXPR)),

/* <expr_logic_or> ::= */
	rrr(EXPR_LOGIC_OR,
		(NT(EXPR_LOGIC_AND)),
		(TK(PIPE_PIPE), NT(EXPR_LOGIC_AND))),
/* <expr_logic_and> ::= */
	rrr(EXPR_LOGIC_AND,
		(NT(EXPR_EQUALITY)),
		(TK(AND_AND), NT(EXPR_EQUALITY))),

/* <expr_equality> ::= */
	rrr(EXPR_EQUALITY,
		(NT(EXPR_COMPARISON)),
		(NT(EXPR_EQUALITY_OP), NT(EXPR_COMPARISON))),
/* <expr_equality_op> ::= */ r(
	TK(EXCL_EQ)
alt	TK(EQ_EQ)
),
/* <expr_comparison> ::= */
	rrr(EXPR_COMPARISON,
		(NT(EXPR_TERM)),
		(NT(EXPR_COMPARISON_OP), NT(EXPR_TERM))),
/* <expr_comparison_op> ::= */ r(
	TK(GT)
alt	TK(GT_EQ)
alt	TK(LT)
alt	TK(LT_EQ)
),

/* <expr_term> ::= */
	rrr(EXPR_TERM,
		(NT(EXPR_FACTOR)),
		(NT(EXPR_TERM_OP), NT(EXPR_FACTOR))),
/* <expr_term_op> ::= */ r(
	TK(MINUS)								/* mult */
alt	TK(PLUS)								/* add */
),
/* <expr_factor> ::= */
	rrr(EXPR_FACTOR,
		(NT(EXPR_UNARY_PREFIX)),
		(NT(EXPR_FACTOR_OP), NT(EXPR_UNARY_PREFIX))),
/* <expr_factor_op> ::= */ r(
	TK(SLASH)								/* div */
alt	TK(STAR)								/* mult */
alt	TK(PERCENT)								/* mod */
),

/* <expr_unary_prefix> ::= */ r(
	NT(EXPR_UNARY_PREFIX_OP), NT(EXPR_UNARY_PREFIX)
alt	NT(EXPR_UNARY_POSTFIX)
),
/* <expr_unary_prefix_op> ::= */ r(
	TK(PLUS)
alt	TK(MINUS)								/* neg op */
alt	TK(EXCL)								/* not op */
),
/* <expr_unary_postfix> ::= */ r(
	NT(EXPR_PRIMARY), NT(EXPR_UNARY_POSTFIX_OPS)
),
/* <expr_unary_postfix_ops> ::= */ r(
	NT(TUPLE), NT(EXPR_UNARY_POSTFIX_OPS)					/* call op */
alt	TK(LSQ_BRACKET), TK(RSQ_BRACKET), NT(EXPR_UNARY_POSTFIX_OPS)		/* index op*/
alt	TK(DOT), TK(IDENT), NT(EXPR_UNARY_POSTFIX_OPS)				/* item op */
alt	EPSILON
),

/* <expr_primary> ::= */ r(
	TK(NUMBER)
alt	TK(INTEGER)
alt	TK(STR)
alt	TK(TRUE)
alt	TK(FALSE)
alt	TK(LPAREN), NT(EXPR), TK(RPAREN)
alt	TK(IDENT)
),

/* <tuple> ::= */ r(
	TK(LPAREN), NT(TUPLE_OPTITEMS), TK(RPAREN)
),
/* <tuple_items> ::= */
	rrr(TUPLE_ITEMS, (NT(EXPR)), (TK(COMMA), NT(EXPR))),
/* <tuple_optitems> ::= */
	ropt(NT(TUPLE_ITEMS)),

/* <decl_fn_def> ::= */ r(
	NT(BLOCK)
alt	TK(SEMI)
),
/* <decl_fn_params> ::= */
	rrr(DECL_FN_PARAMS, (TK(IDENT)), (TK(COMMA), TK(IDENT))),
/* <decl_fn_optparams> ::= */
	ropt(NT(DECL_FN_PARAMS)),

/* <decl_let_def> ::= */
	ropt(TK(EQ), NT(EXPR))
};


void token_destroyer(uint16_t id, void *seminfo)
{
	if (id == TK_IDENT || id == TK_STR)
		str_ref_destroy(((struct seminfo *) seminfo)->seminfo.str_ref);
}
