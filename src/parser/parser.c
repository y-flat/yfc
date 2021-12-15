#include "parser.h"

#include <stdio.h>

#include <api/tokens.h>

int yf_parse(struct yf_lexer * lexer, struct yf_parse_tree * tree) {
    
    /* Temp - print out every token. */
    struct yf_token token;
    for (;;) {
        yfl_lex(lexer, &token);
        if (token.type == YFT_EOF) {
            break;
        }
        printf(
            "%20s, line: %3d, col: %3d, type: %20s\n",
            token.data, token.lineno, token.colno, yf_get_toktype(token.type)
        );
    }

    return 0;

}
