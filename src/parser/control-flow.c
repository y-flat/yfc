#include <parser/parser-internals.h>

int yfp_if(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yfcs_if * i;
    struct yf_token tok;
    int lex_err;

    node->type = YFCS_IF;
    i = &node->ifstmt;

    /* We need an if. */
    P_LEX(lexer, &tok);

    if (tok.type != YFT_IF) {
        YF_TOKERR(tok, "'if'");
    }

    /* Syntax is: if ( expr ) stmt [ else stmt ] */
    /* So get oparen, expr, cparen */
    P_LEX(lexer, &tok);
    if (tok.type != YFT_OPAREN) {
        YF_TOKERR(tok, "'('");
    }

    i->cond = yf_malloc(sizeof (struct yf_parse_node));
    if (!i->cond)
        return 2;

    /* Parse the expression */
    if (yfp_expr(i->cond, lexer, false, NULL)) {
        return 1;
    }

    P_LEX(lexer, &tok);
    if (tok.type != YFT_CPAREN) {
        YF_TOKERR(tok, "')'");
    }

    /* Now parse a body. */
    i->code = yf_malloc(sizeof (struct yf_parse_node));
    if (!i->code)
        return 2;
    if (yfp_stmt(i->code, lexer)) {
        return 1;
    }

    /* Now, see if there's an else clause. */
    P_LEX(lexer, &tok);
    if (tok.type == YFT_ELSE) {
        i->elsebranch = yf_malloc(sizeof (struct yf_parse_node));
        if (!i->elsebranch)
            return 2;
        if (yfp_stmt(i->elsebranch, lexer)) {
            return 1;
        }
    } else {
        yfl_unlex(lexer, &tok);
    }

    return 0;

}
