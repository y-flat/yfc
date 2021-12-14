#include "lexer.h"

/* Forward decls */
static void yfl_core_lex(struct yf_lexer * lexer, struct yf_token * token);

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
    
    if (lexer->unlex_ct > 0) {
        /* We have unlexed tokens, so use them */
        *token = lexer->unlex_buf[lexer->unlex_ct - 1];
        lexer->unlex_ct--;
    } else {
        yfl_core_lex(lexer, token);
    }

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

/**
 * @brief This ACTUALLY does lexing. The yfl_lex function checks the unlexed
 * buffer and returns the top tokens.
 */
static void yfl_core_lex(struct yf_lexer * lexer, struct yf_token * token) {

}
