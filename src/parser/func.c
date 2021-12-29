#include <parser/parser-internals.h>

/**
 * ASSUMES THE FUNCTION NAME AND LEFT PAREN HAVE ALREADY BEEN PARSED.
 */
int yfp_funcdecl(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;
    
    /* TODO - arg parsing */
    /* For now just assumes close paren */
    yfl_lex(lexer, &tok);
    if (tok.type != YFT_CPAREN) {
        YF_TOKERR(tok, "')'");
    }

    /* TODO - parse return type */
    /* Goes straight to function body */

    node->type = YFCS_FUNCDECL;
    node->funcdecl.body = yf_malloc(sizeof (struct yf_parse_node));
    return yfp_bstmt(node->funcdecl.body, lexer);

}
