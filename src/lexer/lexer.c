#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <lexer/keywords.h>
#include <util/yfc-out.h>

/**
 * Get error message from parsing
 */
char * get_error_message(int error_code) {
    static char* yfl_code_message[4] = {
        "Okay",
        "Unknown Error",
        "Open comment",
        "Overflow",
    };

	return yfl_code_message[error_code];
}

/* Forward decls */
static enum yfl_code yfl_core_lex(struct yf_lexer *, struct yf_token *);
static int yfl_getc(struct yf_lexer * lexer);
static int yfl_ungetc(struct yf_lexer * lexer, int c);
static int yfl_skip_whitespace(struct yf_lexer * lexer);
static int yfl_skip_comment(struct yf_lexer * lexer);
static int yfl_skip_all(struct yf_lexer * lexer);

void yfl_init(
    struct yf_lexer * lexer,
    struct yf_lexer_input * input
) {
    
    lexer->loc.line = 1; /* Line count starts at 1 */
    lexer->loc.column = 1;  /* Same with column */
    lexer->loc.file = input->input_name;

    lexer->input = input;
    lexer->unlex_ct = 0;

}

/**
 * Stuff a token with data.
 */
enum yfl_code yfl_lex(struct yf_lexer * lexer, struct yf_token * token) {
    
    if (lexer->unlex_ct > 0) {
        /* We have unlexed tokens, so use them */
        *token = lexer->unlex_buf[lexer->unlex_ct - 1];
        lexer->unlex_ct--;
        return YFLC_OK;
    } else {
        return yfl_core_lex(lexer, token);
    }

}

/**
 * Unlex one token (only up to 16!)
 */
int yfl_unlex(struct yf_lexer * lexer, struct yf_token * token) {
    
    if (lexer->unlex_ct >= 16) {
        return 1;
    }

    lexer->unlex_buf[lexer->unlex_ct] = *token;
    lexer->unlex_ct++;
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
    
    if (isalpha(c) || c == '_') {
        return YFL_IDENT;
    } else if (isdigit(c)) {
        return YFL_NUM;
    } else if (ispunct(c)) {
        return YFL_PUNCT;
    } else if (isspace(c)) {
        return YFL_WHITESPACE;
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

    enum yf_token_type type;
    if ((type = yf_keyword_type(buf)) != YFT_INVALID) {
        return type;
    }

    if (isalpha(buf[0])) return YFT_IDENTIFIER;
    if (isdigit(buf[0])) return YFT_LITERAL;
    if (!strcmp(buf, "::")) return YFT_NAMESPACE;
    if (buf[0] == ';') return YFT_SEMICOLON;
    if (buf[0] == ',') return YFT_COMMA;
    if (buf[0] == ':') return YFT_COLON;
    if (buf[0] == '(') return YFT_OPAREN;
    if (buf[0] == ')') return YFT_CPAREN;
    if (buf[0] == '{') return YFT_OBRACE;
    if (buf[0] == '}') return YFT_CBRACE;
    if (buf[0] == '.') return YFT_DOT;
    if (ispunct(buf[0])) return YFT_OP; /* More */
    return YFT_INVALID;

}

/**
 * This ACTUALLY does lexing. The yfl_lex function checks the unlexed
 * buffer and returns the top tokens, if any.
 */
static enum yfl_code yfl_core_lex(
    struct yf_lexer * lexer, struct yf_token * token
) {

    int startchar, curchar;
    enum yfl_char_type starttype, endconditions;
    int charpos; /* The character being added. */

    /* Skip all irrelevant characters. */
    if (yfl_skip_all(lexer) == -1) return YFLC_OPEN_COMMENT;

    /* Get the start position */
    token->loc = lexer->loc;

    startchar = yfl_getc(lexer);

    /* First, EOF check. */
    if (startchar == EOF) {
        token->type = YFT_EOF;
        strcpy(token->data, "[EOF]");
        lexer->input->close(lexer->input->input);
        return YFLC_OK;
    }
    
    /* Now, get the type of the first character. */
    starttype = yfl_get_type(startchar);
    endconditions = yfl_end_types(starttype);

    charpos = 0;

    curchar = startchar;

    for (;;) {

        token->data[charpos++] = curchar;

        if (charpos >= 256) {
            /* Too big! */
            /* TODO - indicate an error. */
            token->type = YFT_TOO_LARGE;
            return YFLC_OVERFLOW;
        }

        /* Add characters until an end condition is encountered. */
        curchar = yfl_getc(lexer);
        if (yfl_get_type(curchar) & endconditions) {
            /* This is kind of an ugly hack but oh well - if the first character
            is an operator, and this is the next character and is an equals,
            then lex it as one operator, like +=. */
            if (starttype == YFL_PUNCT && curchar == '=' && charpos == 1) {
                continue;
            }
            /* Another such 'ugly hack' for namespaces */
            if (token->data[0] == ':' && curchar == ':' && charpos == 1) {
                continue;
            }
            yfl_ungetc(lexer, curchar);
            token->data[charpos] = '\0';
            token->type = get_type(token->data);
            return YFLC_OK;
        }

    }

}

/**
 * @brief This gets a character, but ALSO increments various properties of the
 * lexer, like line count and such, if needed.
 */
static int yfl_getc(struct yf_lexer * lexer) {

    int c;
    c = lexer->input->getc(lexer->input->input);

    if (c == '\n') {
        ++lexer->loc.line;
        lexer->prev_col = lexer->loc.column;
        lexer->loc.column = 1;
    } else {
        ++lexer->loc.column;
    }

    return c;

}

static int yfl_ungetc(struct yf_lexer * lexer, int c) {

    lexer->input->ungetc(c, lexer->input->input);

    if (c != '\n') {
        --lexer->loc.column;
    } else {
        --lexer->loc.line;
        /* This is why we need the prev_col. If we unlex a newline, we're at
         * the end of the previous line, with an unknown column number. */
        /* Let's just hope we never need to unlex across multiple lines. */
        lexer->loc.column = lexer->prev_col;
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
 * Skip whitespace and comments, if any. Return whether any was skipped. Return
 * -1 if the comment was unclosed.
 */
static int yfl_skip_all(struct yf_lexer * lexer) {

    /**
     * How this works - we try to skip whitespace and comments repeatedly
     * until we try to skip both in a row and neither works.
     */
    int skipped; /* The number of skipped chars */
    int cskipped; /* Comments skipped. */
    for (;;) {
        skipped = 0;
        skipped += yfl_skip_whitespace(lexer);
        cskipped = yfl_skip_comment(lexer);
        if (cskipped == -1) return -1;
        skipped += cskipped;
        if (!skipped)
            return 1;
    }

}

/**
 * Token type representations for debugging purposes.
 */
const char * yf_get_toktype(enum yf_token_type type) {
    static const char * types[] = {
        "!!INVALID!!",
        "end-of-file",
        "identifier",
        "literal",
        "semicolon",
        "comma",
        "colon",
        "opening paren",
        "closing paren",
        "opening brace",
        "closing brace",
        "operator"    ,
        "<return>",
        "<if>",
        "<else>",
        "namespace",
        "[TOO LARGE]",
    };
    
    return types[type];
}
