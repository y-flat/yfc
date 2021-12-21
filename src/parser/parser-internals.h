#ifndef PARSER_PARSER_INTERNALS_H
#define PARSER_PARSER_INTERNALS_H

static int yfp_program(struct yf_parse_node * node, struct yf_lexer * lexer);
static int yfp_vardecl(struct yf_parse_node * node, struct yf_lexer * lexer);
static int yfp_funcdecl(struct yf_parse_node * node, struct yf_lexer * lexer);
static int yfp_stmt(struct yf_parse_node * node, struct yf_lexer * lexer);
static int yfp_expr(struct yf_parse_node * node, struct yf_lexer * lexer);
static int yfp_ident(struct yfcs_identifier * node, struct yf_lexer * lexer);
static int yfp_type(struct yfcs_type * node, struct yf_lexer * lexer);

#endif /* PARSER_PARSER_INTERNALS_H */
