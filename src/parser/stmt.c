#include <parser/parser-internals.h>

#include <string.h>

int yfp_stmt(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;
    struct yf_parse_node ident;
    int lex_err, ret;
    bool expect_semicolon;

    P_LEX(lexer, &tok);

    /**
     * A statement can begin one of several ways:
     * "{" [ ... block statement ]
     * "identifier" "op" [ ... expression]
     * "identifier" "colon" [ ... vardecl ]
     * "(" [ ... expression] ")"
     */
    expect_semicolon = 1; /* Probably redundant */
    switch (tok.type) {
        case YFT_OBRACE:
            yfl_unlex(lexer, &tok);
            ret = yfp_bstmt(node, lexer);
            expect_semicolon = false;
            goto out;
        case YFT_IDENTIFIER:
            expect_semicolon = true;
            /* So here, it's either a vardecl or an expr. We don't know, and we
            can't unlex a whole identifier, so we check the next token and enter
            the appropriate parsing routine "in the middle". */
            yfl_unlex(lexer, &tok);
            if (yfp_ident(&ident.as.expr.as.value.as.identifier, lexer))
                return 1;
            yfl_lex(lexer, &tok);
            if (tok.type == YFT_COLON) {
                /* TODO - reduce the copied code */
                node->as.vardecl.name.name.datalen =
                    ident.as.expr.as.value.as.identifier.name.datalen;
                strncpy(
                    node->as.vardecl.name.name.databuf,
                    ident.as.expr.as.value.as.identifier.name.databuf,
                    node->as.vardecl.name.name.datalen
                );
                if (yfp_vardecl(node, lexer)) {
                    return 1;
                }
            } else if (tok.type == YFT_OP) {
                yfl_unlex(lexer, &tok);
                ret = yfp_expr(node, lexer, true, &ident);
                goto out;
            } else {
                YF_TOKERR(tok, "':' or operator");
            }
        case YFT_OPAREN:
            yfl_unlex(lexer, &tok);
            ret = yfp_expr(node, lexer, 0, NULL);
            goto out;
        default:
            YF_TOKERR(tok, "identifier, '{', or '('");
    }

out:
    if (expect_semicolon) {
        yfl_lex(lexer, &tok);
        if (tok.type != YFT_SEMICOLON) {
            YF_TOKERR(tok, "';'");
        }
    }

    return ret;

}
