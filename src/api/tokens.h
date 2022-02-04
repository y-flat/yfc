/**
 * Token definitions and types.
 */

#ifndef API_TOKENS_H
#define API_TOKENS_H

#include <api/loc.h>

enum yf_token_type {

    /* Not much yet */

    YFT_INVALID,

    YFT_EOF,
    YFT_IDENTIFIER, /* Types or not */
    YFT_LITERAL,
    YFT_SEMICOLON,
    YFT_COMMA,
    YFT_COLON,  /* : */
    YFT_OPAREN, /* ( */
    YFT_CPAREN, /* ) */
    YFT_OBRACE, /* { */
    YFT_CBRACE, /* } */
    YFT_OP    , /* =, etc. */
    YFT_RETURN, /* return */
    YFT_IF,
    YFT_ELSE,
    YFT_TOO_LARGE, /* A token was too large */

};

/**
 * TODO - have source file name
 */
struct yf_token {

    enum yf_token_type type;

    /* Maybe increase this later, for long strings? */
    char data [ 256 ];

    struct yf_location loc;

};

#endif /* API_TOKENS_H */
