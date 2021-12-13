/**
 * Lexer for Y-flat. Provides one function only - lexing the next token.
 * Oh, and unlexing.
 */

#ifndef LEXER_LEXER_H
#define LEXER_LEXER_H

#include <api/lexer-input.h>
#include <api/tokens.h>

struct yf_lexer {

    /* Current parsing data */
    int line, col;

    struct yf_lexer_input * input;

    struct yf_token unlex_buf[16];
    int unlex_ct;

};

/**
 * Stuff a token with data.
 */
void yfl_lex(struct yf_lexer * lexer, struct yf_token * token);

/**
 * Unlex a token, up to 16.
 */

#endif /* LEXER_LEXER_H */
