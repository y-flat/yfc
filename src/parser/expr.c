/**
 * Expression parsing. This algorithm is complicated enough that it deserves its
 * own file. Yikes!
 */

#include <string.h>

#include <parser/parser-internals.h>

int yfp_expr(struct yf_parse_node * node, struct yf_lexer * lexer) {
    
    struct yf_token tok;

    node->type = YFCS_EXPR;
    
    yfl_lex(lexer, &tok);
    /* TODO - handle recursive exprs */
    switch (tok.type) {
    case YFT_IDENTIFIER:
        yfl_unlex(lexer, &tok);
        yfp_ident(&node->as.expr.value.as.identifier, lexer);
        break;
    case YFT_LITERAL:
        strcpy(
            node->as.expr.value.as.literal.value.databuf, tok.data
        );
        node->as.expr.value.type = YFCS_LITERAL;
        break;
    default:
        YF_TOKERR(tok, "identifier or literal");
        break;
    }

    return 0;

}
