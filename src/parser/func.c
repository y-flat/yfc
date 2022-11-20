#include <parser/parser-internals.h>

#include <string.h>

/**
 * ASSUMES THE FUNCTION NAME AND LEFT PAREN HAVE ALREADY BEEN PARSED.
 */
int yfp_funcdecl(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;
    struct yf_parse_node * argp; /* Argument pointer - just used as a temp to
    parse and then pass to parse_vardecl */
    struct yfcs_identifier ident;
    int argct; /* Needed only to check for commas correctly. */

    int lex_err;

    /* No extc -- yet. */
    node->funcdecl.extc = false;

    /* Start arg list for writing */
    if (yf_list_init(&node->funcdecl.params) != YF_OK)
        abort();
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

        /* Now, parse ident and colon, then enter vardecl. */
        if (yfp_ident(&ident, lexer)) {
            return 1;
        }
        argp = yf_malloc(sizeof(struct yf_parse_node));
        if (!argp) {
            return 1;
        }
        P_GETCT(argp, ident);
        P_LEX(lexer, &tok);
        if (tok.type != YFT_COLON) {
            YF_TOKERR(tok, "':' following argument name");
        }
        argp->vardecl.name = ident;
        if (yfp_vardecl(argp, lexer)) {
            free(argp);
            return 1;
        }

        ++argct;

        /* Add to arg list */
        if (yf_list_add(&node->funcdecl.params, argp) != YF_OK)
            abort();

    }

    /* Parse return type, which is colon type (a lack of this means "void") */
    P_LEX(lexer, &tok);
    if (tok.type != YFT_COLON) {
        if (tok.type != YFT_OBRACE) {
            /* Expect function body */
            YF_PRINT_ERROR(
                "Expected ':' or '{' following function declaration, got '%s'",
                tok.data
            );
        }
        strcpy(node->funcdecl.ret.databuf, "void");
        P_GETCT(&node->funcdecl.ret, tok);
        /* Unlex opening brace */
        yfl_unlex(lexer, &tok);
        goto bodyp;
    }

    /* Parse return type */
    if (yfp_type(&node->funcdecl.ret, lexer)) {
        return 1;
    }

    /* Goes straight to function body */

bodyp:
    node->type = YFCS_FUNCDECL;

    struct yf_token sctest; /* semicolon test */
semicolon:
    yfl_lex(lexer, &sctest);
    switch (sctest.type) {
    case YFT_EXTC:
        if (!node->funcdecl.extc) {
            node->funcdecl.extc = true;
            /* We need to continue parsing like before, checking for ';' and all. So ... */
            /* This is why the check below exists at all. */
            goto semicolon;
        } else {
            YF_TOKERR(sctest, "body or semicolon");
            /* TODO -- be able to print this message. The above call returns. */
            /* YF_PRINT_ERROR("note: only one `extc` modifier is allowed."); */
        }
    case YFT_SEMICOLON:
        /* No body for function. */
        node->funcdecl.body = NULL;
        return 0;
    default:
        yfl_unlex(lexer, &sctest);
    }

    node->funcdecl.body = yf_malloc(sizeof (struct yf_parse_node));
    return yfp_bstmt(node->funcdecl.body, lexer);

}
