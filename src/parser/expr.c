/**
 * Expression parsing. This algorithm is complicated enough that it deserves its
 * own file. Yikes!
 */

#include <string.h>

#include <parser/parser-internals.h>

/**
 * So that there's ZERO confusion for future me or others, here's what this does
 * - it parses an expression that cannot EXPLICITLY be divided into two parts.
 * For example, if we're parsing:
 *  (a + b) * c - d + e
 * The "atomic expressions" are (a + b), c, d, and e.
 * The reason parenthesized expressions are atomic is because the expression
 * tree is made by organizing expressions by operator precedence, and we can't
 * split up expressions in parentheses.
 */
int yfp_atomic_expr(struct yf_parse_node * node, struct yf_lexer * lexer) {
    
    struct yf_token tok;

    node->type = YFCS_EXPR;
    
    yfl_lex(lexer, &tok);
    switch (tok.type) {
    case YFT_IDENTIFIER:
        yfl_unlex(lexer, &tok);
        yfp_ident(&node->as.expr.as.value.as.identifier, lexer);
        break;
    case YFT_LITERAL:
        strcpy(
            node->as.expr.as.value.as.literal.value.databuf, tok.data
        );
        node->as.expr.as.value.type = YFCS_LITERAL;
        break;
    default:
        YF_TOKERR(tok, "identifier or literal");
        break;
    }

    return 0;

}

int yfp_expr(struct yf_parse_node * node, struct yf_lexer * lexer) {

    /* TODO - parse multiple branches */
    return yfp_atomic_expr(node, lexer);

}
