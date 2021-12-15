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

    /* There's an explanation for this in yfl_ungetc */
    int prev_col;

    struct yf_lexer_input * input;

    struct yf_token unlex_buf[16];
    int unlex_ct;

};

/**
 * Init a lexer.
 */
void yfl_init(struct yf_lexer * lexer, struct yf_lexer_input * input);

/**
 * Stuff a token with data.
 */
void yfl_lex(struct yf_lexer * lexer, struct yf_token * token);

/**
 * Unlex one token (only up to 16!) Return 1 if failed.
 */
int yfl_unlex(struct yf_lexer * lexer, struct yf_token * token);

/**
 * Get the string of a token type.
 */
const char * yf_get_toktype(enum yf_token_type type);

/**
 * Unlex a token, up to 16.
 */

#endif /* LEXER_LEXER_H */
