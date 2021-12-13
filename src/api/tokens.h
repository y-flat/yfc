/**
 * Token definitions and types.
 */

#ifndef API_TOKENS_H
#define API_TOKENS_H

enum yf_token_type {

    /* Not much yet */

    YFT_EOF,
    YFT_IDENTIFIER, /* NOT types */
    YFT_TYPE, /* YES types */
    YFT_LITERAL,
    YFT_SEMICOLON,
    YFT_COMMA,
    YFT_COLON,  /* : */
    YFT_OPAREN, /* ( */
    YFT_CPAREN, /* ) */
    YFT_OBRACE, /* { */
    YFT_CBRACE, /* } */

};

struct yf_token {

    enum yf_token_type type;

    /* Maybe increase this later, for long strings? */
    char data [ 256 ];

    int lineno;
    int colno;

};

#endif /* API_TOKENS_H */
