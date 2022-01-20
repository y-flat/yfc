#include <parser/parser-internals.h>

#include <string.h>

int yfp_stmt(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;
    struct yf_parse_node ident;
    int lex_err, ret;
    bool expect_semicolon;

    P_PEEK(lexer, &tok);
    P_GETCT(node, tok);

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
            ret = yfp_bstmt(node, lexer);
            expect_semicolon = false;
            goto out;
        case YFT_IDENTIFIER:
            expect_semicolon = true;
            /* So here, it's either a vardecl or an expr. We don't know, and we
            can't unlex a whole identifier, so we check the next token and enter
            the appropriate parsing routine "in the middle". */
            if (yfp_ident(&ident.expr.value.identifier, lexer))
                return 1;
            P_LEX(lexer, &tok);
            if (tok.type == YFT_COLON) {
                /* TODO - reduce the copied code */
                strcpy(
                    node->vardecl.name.name,
                    ident.expr.value.identifier.name
                );
                ret = yfp_vardecl(node, lexer);
                goto out;
            /* Expression or funccall */
            } else if (tok.type == YFT_OP || tok.type == YFT_OPAREN) {
                ret = yfp_expr(node, lexer, true, &ident);
                goto out;
            } else {
                YF_TOKERR(tok, "':' or operator");
            }
        case YFT_OPAREN:
            ret = yfp_expr(node, lexer, 0, NULL);
            goto out;
        case YFT_RETURN:
            P_LEX(lexer, &tok);
            node->type = YFCS_RET;
            /* Just parse an expression. */
            /* But a semicolon alone is okay. */
            P_PEEK(lexer, &tok);
            if (tok.type == YFT_SEMICOLON) {
                ret = 0;
                node->ret.expr = NULL;
            } else {
                node->ret.expr = yf_malloc(sizeof(struct yf_parse_node));
                if (!node->ret.expr)
                    return 1;
                ret = yfp_expr(node->ret.expr, lexer, false, NULL);
            }
            goto out;
        case YFT_IF:
            ret = yfp_if(node, lexer);
            expect_semicolon = false;
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

/**
 * Assumes the opening identifier has already been lexed.
 */
int yfp_funccall(struct yf_parse_node * node, struct yf_lexer * lexer) {

    int lex_err, argct;
    struct yf_token tok;
    struct yf_parse_node * argp;

    P_LEX(lexer, &tok);
    if (tok.type != YFT_OPAREN) {
        YF_TOKERR(tok, "'('");
    }

    /**
     * THIS CODE IS VERY SIMILAR TO THE STUFF FOR PARSING FUNCDECLS.
     */

    /* Start arg list for writing */
    yf_list_init(&node->expr.call.args);
    argct = 0;

    for (;;) {

        P_LEX(lexer, &tok);

        /* Close paren check */
        if (tok.type == YFT_CPAREN) {
            break; /* Done arg parsing */
        }

        if (argct > 0) {
            if (tok.type != YFT_COMMA) {
                YF_TOKERR(tok, "',' following argument");
            }
        } else {
            yfl_unlex(lexer, &tok);
        }

        argp = yf_malloc(sizeof(struct yf_parse_node));
        if (!argp) {
            return 1;
        }

        if (yfp_expr(argp, lexer, 0, NULL)) {
            return 1;
        }

        ++argct;

        /* Add to arg list */
        yf_list_add(&node->expr.call.args, argp);

    }

    return 0;

}
