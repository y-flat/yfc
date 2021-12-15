#include "lexer.h"

/* Forward decls */
static void yfl_core_lex(struct yf_lexer * lexer, struct yf_token * token);
static int yfl_getc(struct yf_lexer * lexer);
static int yfl_ungetc(struct yf_lexer * lexer, int c);

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
 * This ACTUALLY does lexing. The yfl_lex function checks the unlexed
 * buffer and returns the top tokens, if any.
 */
static void yfl_core_lex(struct yf_lexer * lexer, struct yf_token * token) {

}

/**
 * @brief This gets a character, but ALSO increments various properties of the
 * lexer, like line count and such, if needed.
 */
static int yfl_getc(struct yf_lexer * lexer) {

    int c;
    c = lexer->input->getc(lexer->input);

    if (c == '\n') {
        ++lexer->line;
        lexer->prev_col = lexer->col;
        lexer->col = 1;
    } else {
        ++lexer->col;
    }

    return 0;

}

static int yfl_ungetc(struct yf_lexer * lexer, int c) {

    lexer->input->ungetc(c, lexer->input);

    if (c != '\n') {
        --lexer->col;
    } else {
        --lexer->line;
        /* This is why we need the prev_col. If we unlex a newline, we're at
         * the end of the previous line, with an unknown column number. */
        /* Let's just hope we never need to unlex across multiple lines. */
        lexer->col = lexer->prev_col;
    }

    return 0;

}
