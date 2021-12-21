#include "parser.h"

#include <stdio.h>
#include <string.h>

#include <api/tokens.h>
#include <lexer/lexer.h>
#include <parser/parser-internals.h>
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

int yf_parse(struct yf_lexer * lexer, struct yf_parse_node * tree) {
    
    return yfp_program(tree, lexer);

}

/**
 * Parse program - check whether we're parsing a vardecl or a funcdecl, then
 * parse one of those, forever.
 */
static int yfp_program(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;
    struct yfcs_identifier ident;
    int lex_err;
    /* All decls are stuffed in here, and then added. */
    struct yf_parse_node * decl;

    node->type = YFCS_PROGRAM;
    yf_list_init(&node->as.program.decls);

    for (;;) {

        decl = yf_malloc(sizeof(struct yf_parse_node));

        /* Variable decls look like this to start:
         * [identifier] : [type] ...
         * And function decls like:
         * [identifier] ( [identifier] ...
         * So we just parse an identifier and see whether the next token is a
         * colon or a left paren.
         */

        /* Do end-of-file peek back here. */
        lex_err = yfl_lex(lexer, &tok);
        if (tok.type == YFT_EOF) {
            free(decl);
            return 0;
        } else {
            yfl_unlex(lexer, &tok);
        }
        
        yfp_ident(&ident, lexer);

        lex_err = yfl_lex(lexer, &tok);
        switch (tok.type) {
            case YFT_COLON:
                decl->as.vardecl.name.name.datalen = ident.name.datalen;
                strncpy(
                    decl->as.vardecl.name.name.databuf, ident.name.databuf,
                    decl->as.vardecl.name.name.datalen
                );
                yfp_vardecl(decl, lexer);
                /* It's a top-level decl, so expect a semicolon. */
                lex_err = yfl_lex(lexer, &tok);
                if (tok.type != YFT_SEMICOLON) {
                    YF_TOKERR(tok, "semicolon");
                }
                break;
            case YFT_OPAREN:
                decl->as.funcdecl.name.name.datalen = ident.name.datalen;
                strncpy(
                    decl->as.funcdecl.name.name.databuf, ident.name.databuf,
                    decl->as.funcdecl.name.name.datalen
                );
                yfp_funcdecl(decl, lexer);
                break;
            default:
                YF_TOKERR(tok, "colon or left paren");
                break;
        }

        /* Now, we have a node - add it to the list. */
        yf_list_add(&node->as.program.decls, decl);

    }

}

/**
 * ASSUMES THE VARIABLE NAME AND COLON HAVE ALREADY BEEN PARSED.
 */
static int yfp_vardecl(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;

    node->type = YFCS_VARDECL;
    
    /* We've parsed all of this: [name] colon */
    /* So now, we expect a type. */
    yfp_type(&node->as.vardecl.type, lexer);

    /**
     * If there's an equal sign, we have an initializer.
     * Otherwise, don't worry about any of it, because vardecls can be nested
     * in other things and practically any token could follow.
     */
    yfl_lex(lexer, &tok);
    switch (tok.type) {
        case YFT_OP: /* TODO - do an equals sign check */
            node->as.vardecl.expr = yf_malloc(sizeof(struct yf_parse_node));
            yfp_expr(node->as.vardecl.expr, lexer);
            break;
        default:
            /* Unlex unimportant token. */
            yfl_unlex(lexer, &tok);
            break;
    }

    return 0;

}

/**
 * ASSUMES THE FUNCTION NAME AND LEFT PAREN HAVE ALREADY BEEN PARSED.
 */
static int yfp_funcdecl(struct yf_parse_node * node, struct yf_lexer * lexer) {
    return 0; /* TODO */
}

static int yfp_stmt(struct yf_parse_node * node, struct yf_lexer * lexer) {
    return 0; /* TODO */
}

static int yfp_expr(struct yf_parse_node * node, struct yf_lexer * lexer) {
    
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

static int yfp_ident(struct yfcs_identifier * node, struct yf_lexer * lexer) {
    /* TODO - parse compound names like "foo.bar.baz" */
    struct yf_token tok;
    yfl_lex(lexer, &tok);
    if (tok.type != YFT_IDENTIFIER) {
        YF_TOKERR(tok, "identifier");
    } else {
        node->name.datalen = strlen(tok.data);
        strcpy(node->name.databuf, tok.data);
    }
    return 0;
}

static int yfp_type(struct yfcs_type * node, struct yf_lexer * lexer) {
    /* TODO - parse compound types */
    struct yf_token tok;
    yfl_lex(lexer, &tok);
    if (tok.type != YFT_IDENTIFIER) {
        YF_TOKERR(tok, "identifier");
    } else {
        node->datalen = strlen(tok.data);
        strcpy(node->databuf, tok.data);
    }
    return 0;
}
