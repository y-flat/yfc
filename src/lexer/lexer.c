#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* Forward decls */
static void yfl_core_lex(struct yf_lexer * lexer, struct yf_token * token);
static int yfl_getc(struct yf_lexer * lexer);
static int yfl_ungetc(struct yf_lexer * lexer, int c);
static int yfl_skip_whitespace(struct yf_lexer * lexer);
static int yfl_skip_comment(struct yf_lexer * lexer);
static int yfl_skip_all(struct yf_lexer * lexer);

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
 * Now, here's the way the lexer works. The first character of a token will
 * determine what type of token it is (ident, number, etc.), and also what
 * characters can end it. So we have functions that determine possible end types
 * from a beginning type, and then we consume characters until we reach an end.
 */

enum yfl_char_type {
    /* Just a note - these are NOT the same as token types. */
    YFL_UNKNOWN     = 0x00000000,
    YFL_IDENT       = 0x00000001,
    YFL_NUM         = 0x00000010,
    YFL_PUNCT       = 0x00000100,
    YFL_WHITESPACE  = 0x00001000,
    YFL_EOF         = 0x00010000,
};

static enum yfl_char_type yfl_get_type(int c) {
    
    if (isalpha(c)) {
        return YFL_IDENT;
    } else if (isdigit(c)) {
        return YFL_NUM;
    } else if (ispunct(c)) {
        return YFL_PUNCT;
    } else if (c == EOF) {
        return YFL_EOF;
    } else {
        return YFL_UNKNOWN;
    }

}

/**
 * Get a bit field of all characters which end a token.
 */
static enum yfl_char_type yfl_end_types(enum yfl_char_type type) {

    switch (type) {
        /* There should be an error for this. */
        case YFL_UNKNOWN:
            return YFL_UNKNOWN;
        case YFL_IDENT:
            return YFL_PUNCT | YFL_WHITESPACE | YFL_EOF;
        case YFL_NUM:
            return YFL_PUNCT | YFL_WHITESPACE | YFL_EOF;
        case YFL_PUNCT:
            return YFL_IDENT | YFL_NUM | YFL_PUNCT | YFL_WHITESPACE | YFL_EOF;
        /* Should also never happen. */
        case YFL_WHITESPACE:
        case YFL_EOF:
            return YFL_UNKNOWN;
    }

}

/**
 * Get the token type from a charbuf.
 */
static enum yf_token_type get_type(char * buf) {

    if (isalpha(buf[0])) return YFT_IDENTIFIER;
    if (isdigit(buf[0])) return YFT_LITERAL;
    if (buf[0] == ';') return YFT_COLON;
    if (buf[0] == ',') return YFT_COMMA;
    if (buf[0] == ':') return YFT_COLON;
    if (buf[0] == '(') return YFT_OPAREN;
    if (buf[0] == ')') return YFT_CPAREN;
    if (buf[0] == '{') return YFT_OBRACE;
    if (buf[0] == '}') return YFT_CBRACE;
    return YFT_INVALID;

}

/**
 * This ACTUALLY does lexing. The yfl_lex function checks the unlexed
 * buffer and returns the top tokens, if any.
 */
static void yfl_core_lex(struct yf_lexer * lexer, struct yf_token * token) {

    int startchar, curchar;
    enum yfl_char_type starttype, endconditions;
    int charpos; /* The character being added. */

    /* Skip all irrelevant characters. */
    yfl_skip_all(lexer);

    /* Get the start position */
    token->lineno = lexer->line;
    token->colno  = lexer->col;

    startchar = yfl_getc(lexer);

    /* First, EOF check. */
    if (startchar == EOF) {
        token->type = YFT_EOF;
        strcpy(token->data, "[EOF]");
        return;
    }
    
    /* Now, get the type of the first character. */
    starttype = yfl_get_type(startchar);
    endconditions = yfl_end_types(starttype);

    charpos = 0;

    for (;;) {

        if (charpos >= 256) {
            /* Too big! */
            /* TODO - indicate an error. */
            token->type = YFT_TOO_LARGE;
            break;
        }

        /* Add characters until an end condition is encountered. */
        curchar = yfl_getc(lexer);
        if (yfl_get_type(curchar) & endconditions) {
            yfl_ungetc(lexer, curchar);
            token->data[charpos] = '\0';
            token->type = get_type(token->data);
            return;
        }

        token->data[charpos++] = curchar;

    }

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

/**
 * Skip whitespace, if any. Return whether any was skipped.
 */
static int yfl_skip_whitespace(struct yf_lexer * lexer) {

    int c;
    int skipped; /* The number of whitespace chars skipped */

    skipped = 0;

    while (1) {
        c = yfl_getc(lexer);
        if (!isspace(c)) {
            yfl_ungetc(lexer, c);
            break;
        }
        ++skipped;
    }

    /* Check the count. */
    return skipped != 0;

}

/**
 * Skip one comment, if any. Return whether any was skipped. Return -1 if the
 * comment was unclosed.
 */
static int yfl_skip_comment(struct yf_lexer * lexer) {

    int c;
    int skipped; /* The number of comment chars skipped */

    skipped = 0;

    /* Comments are delimited by two tildes on either side. */
    c = yfl_getc(lexer);
    if (c != '~') {
        yfl_ungetc(lexer, c);
        return 0;
    }
    c = yfl_getc(lexer);
    if (c != '~') {
        /* We're on the second char, so unlex both. */
        yfl_ungetc(lexer, c);
        yfl_ungetc(lexer, '~');
        return 0;
    }

    skipped = 2; /* ~, and then ~ */

    /* Now we go through until we reach either two tildes or a file end. */
    for (;;) {
        c = yfl_getc(lexer);
        if (c == '~') {
            /* Is it the end of the comment??? */
            c = yfl_getc(lexer);
            if (c == '~') {
                /* Yes, it is. */
                skipped += 2;
                break;
            } else {
                /* No, it's just a tilde alone. We were TRICKED! */
                yfl_ungetc(lexer, c);
                /* Don't forget to unlex the tilde. */
                yfl_ungetc(lexer, '~');
                skipped += 1;
            }
        }
        if (c == EOF) {
            /* We hit the end of the file. */
            return -1;
        }
    }

    /* Check the count. */
    return skipped != 0;

}

/**
 * Skip whitespace and comments, if any. Return whether any was skipped.
 */
static int yfl_skip_all(struct yf_lexer * lexer) {

    /**
     * How this works - we try to skip whitespace and comments repeatedly
     * until we try to skip both in a row and neither works.
     */
    int skipped; /* The number of skipped chars */
    for (;;) {
        skipped = 0;
        skipped += yfl_skip_whitespace(lexer);
        skipped += yfl_skip_comment(lexer);
        if (!skipped)
            return 1;
    }

}
