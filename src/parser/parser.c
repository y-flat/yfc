#include "parser.h"

#include <stdio.h>
#include <string.h>

#include <parser/parser-internals.h>

int yf_parse(struct yf_lexer * lexer, struct yf_parse_node * tree) {
    
    return yfp_program(tree, lexer);

}

/**
 * Parse program - check whether we're parsing a vardecl or a funcdecl, then
 * parse one of those, forever.
 */
int yfp_program(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;
    struct yfcs_identifier ident;
    int lex_err;
    /* All decls are stuffed in here, and then added. */
    struct yf_parse_node * decl;

    /* Unimportant */
    node->loc.line = node->loc.column = -1;

    node->type = YFCS_PROGRAM;
    yf_list_init(&node->program.decls);

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
        P_PEEK(lexer, &tok);
        if (tok.type == YFT_EOF) {
            free(decl);
            return 0;
        }
        
        yfp_ident(&ident, lexer);   
        P_GETCT(decl, ident);

        P_LEX(lexer, &tok);
        switch (tok.type) {
            case YFT_COLON:
                strcpy(
                    decl->vardecl.name.name, ident.name
                );
                if (yfp_vardecl(decl, lexer)) {
                    free(decl);
                    return 1;
                }
                /* It's a top-level decl, so expect a semicolon. */
                P_LEX(lexer, &tok);
                if (tok.type != YFT_SEMICOLON) {
                    YF_TOKERR(tok, "semicolon");
                }
                break;
            case YFT_OPAREN:
                strcpy(
                    decl->funcdecl.name.name, ident.name
                );
                if (yfp_funcdecl(decl, lexer)) {
                    free(decl);
                    return 1;
                }
                break;
            default:
                YF_TOKERR(tok, "colon or left paren");
                break;
        }

        /* Now, we have a node - add it to the list. */
        yf_list_add(&node->program.decls, decl);

    }

}

/**
 * ASSUMES THE VARIABLE NAME AND COLON HAVE ALREADY BEEN PARSED.
 */
int yfp_vardecl(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;
    int lex_err;

    node->type = YFCS_VARDECL;

    /**
     * The lineno and colno are set to the same as the identifier, so not here.
     */
    
    /* We've parsed all of this: [name] colon */
    /* So now, we expect a type. */
    if (yfp_type(&node->vardecl.type, lexer)) {
        return 1;
    }

    /**
     * If there's an equal sign, we have an initializer.
     * Otherwise, don't worry about any of it, because vardecls can be nested
     * in other things and practically any token could follow.
     */
    P_LEX(lexer, &tok);
    switch (tok.type) {
        case YFT_OP:
        if (strcmp(tok.data, "=")) {
            YF_TOKERR(tok, "equal sign");
        }
            node->vardecl.expr = yf_malloc(sizeof(struct yf_parse_node));
            if (yfp_expr(node->vardecl.expr, lexer, 0, NULL)) {
                free(node->vardecl.expr);
                return 1;
            }
            break;
        default:
            /* Unlex unimportant token. */
            yfl_unlex(lexer, &tok);
            /* Also - expression is NULL. */
            node->vardecl.expr = NULL;
            break;
    }

    return 0;

}

/**
 * How this works:
 * We copy data into the prefix buffer until we stop encountering a sequence of
 * identifier - dot - identifier - dot ...
 * If it's a namespace separator, we start parsing the actual name. Otherwise,
 * we copy the "prefix" into the "actual" name and break.
 */
int yfp_ident(struct yfcs_identifier * node, struct yf_lexer * lexer) {
   
    int lex_err;
    struct yf_token tok;

    P_PEEK(lexer, &tok);
    P_GETCT(node, tok);

    P_LEX(lexer, &tok);
    if (tok.type != YFT_IDENTIFIER) {
        YF_TOKERR(tok, "identifier");
    } else {
        strcpy(node->filepath, tok.data);
    }

    /* Go through the dot - identifier loop. */
    for (;;) {
        
        P_LEX(lexer, &tok);
        /* Either a dot or a namespace separator. */
        switch (tok.type) {
        case YFT_DOT:
            /* Copy the dot into the prefix. */
            strcat(node->filepath, tok.data);
            goto cont;
        case YFT_NAMESPACE:
            goto parse_name;
        default:
            /* Unlex unimportant token. */
            yfl_unlex(lexer, &tok);
            /* There's no prefix. */
            strcpy(node->name, node->filepath);
            strcpy(node->filepath, lexer->input->input_name);
            goto done;
        }

        P_LEX(lexer, &tok);
        /* Should be an identifier. */
        if (tok.type != YFT_IDENTIFIER) {
            YF_TOKERR(tok, "identifier");
        } else {
            strcat(node->filepath, tok.data);
        }

cont:   ;

    }

parse_name:
    /* Go through the dot - identifier loop. Similar code to above. */
    for (;;) {
        
        P_LEX(lexer, &tok);
        /* Either a dot or a namespace separator. */
        switch (tok.type) {
        case YFT_DOT:
            /* Copy the dot into the prefix. */
            strcat(node->name, tok.data);
            goto cont2;
        case YFT_NAMESPACE:
            YF_PRINT_ERROR("Multiple namespace separators are not "
                "allowed.");
        default:
            /* Unlex unimportant token. */
            yfl_unlex(lexer, &tok);
            goto done;
        }

        P_LEX(lexer, &tok);
        /* Should be an identifier. */
        if (tok.type != YFT_IDENTIFIER) {
            YF_TOKERR(tok, "identifier");
        } else {
            strcat(node->name, tok.data);
        }

cont2:   ;

    }

done:
    return 0;

}

int yfp_type(struct yfcs_type * node, struct yf_lexer * lexer) {
    int lex_err;
    /* TODO - parse compound types */
    struct yf_token tok;
    P_LEX(lexer, &tok);
    P_GETCT(node, tok);
    if (tok.type != YFT_IDENTIFIER) {
        YF_TOKERR(tok, "identifier");
    } else {
        strcpy(node->databuf, tok.data);
    }
    return 0;
}

int yfp_bstmt(struct yf_parse_node * node, struct yf_lexer * lexer) {

    struct yf_token tok;
    struct yf_parse_node * stmt;
    int lex_err;

    /* '{' [ statements ] '}' */

    P_LEX(lexer, &tok);
    if (tok.type != YFT_OBRACE) {
        YF_TOKERR(tok, "'{'");
    }

    P_GETCT(node, tok);

    node->type = YFCS_BSTMT;
    yf_list_init(&node->bstmt.stmts);

    for (;;) {
        P_PEEK(lexer, &tok);
        if (tok.type == YFT_CBRACE) {
            /* Consume */
            P_LEX(lexer, &tok);
            return 0;
        }
        stmt = yf_malloc(sizeof (struct yf_parse_node));
        if (yfp_stmt(stmt, lexer)) {
            free(stmt);
            return 1;
        }
        yf_list_add(&node->bstmt.stmts, stmt);
    }

}
