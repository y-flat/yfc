#include "lexer.h"

void yfl_init(struct yf_lexer * lexer, struct yf_lexer_input * input) {
    
    lexer->line = 1; /* Line count starts at 1 */
    lexer->col = 1;  /* Same with column */

    lexer->input = input;
    lexer->unlex_ct = 0;

}

/**
 * Stuff a token with data.
 */
void yfl_lex(struct yf_lexer * lexer, struct yf_token * token) {
    /* TODO */
}

/**
 * Unlex one token (only up to 16!)
 */
int yfl_unlex(struct yf_lexer * lexer, struct yf_token * token) {
    
    if (lexer->unlex_ct >= 16) {
        return 1;
    }

    lexer->unlex_ct++;
    lexer->unlex_buf[lexer->unlex_ct] = *token;
    return 0;

}

