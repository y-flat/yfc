#ifndef PARSER_PARSER_INTERNALS_H
#define PARSER_PARSER_INTERNALS_H

/* All includes for every parser file */

#include <api/concrete-tree.h>
#include <api/tokens.h>
#include <lexer/lexer.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

/**
 * Print error if token type unexpected.
 * Example: YF_TOKERR(tok, "comma or colon")
 */
#define YF_TOKERR(tok, expected) do { \
    YF_PRINT_ERROR( \
        "Unexpected token %s, line %d column %d:" \
        "expected %s, found token of type \"%s\"", \
        tok.data, \
        tok.lineno, \
        tok.colno, \
        expected, \
        yf_get_toktype(tok.type) \
    ); \
    return 4; /* TODO */ \
} while (0)

int yfp_program(struct yf_parse_node * node, struct yf_lexer * lexer);
int yfp_vardecl(struct yf_parse_node * node, struct yf_lexer * lexer);
int yfp_funcdecl(struct yf_parse_node * node, struct yf_lexer * lexer);
int yfp_stmt(struct yf_parse_node * node, struct yf_lexer * lexer);
int yfp_expr(struct yf_parse_node * node, struct yf_lexer * lexer);
int yfp_ident(struct yfcs_identifier * node, struct yf_lexer * lexer);
int yfp_type(struct yfcs_type * node, struct yf_lexer * lexer);

#endif /* PARSER_PARSER_INTERNALS_H */
