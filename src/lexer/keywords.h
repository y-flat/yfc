/**
 * A utility for determining keywords.
 */

#ifndef LEXER_KEYWORDS_H
#define LEXER_KEYWORDS_H

#include <api/tokens.h>

/**
 * Get the keyword type efrom a keyword, or YFT_INVALID if it is not a keyword.
 */
enum yf_token_type yf_keyword_type(char * str);

#endif /* LEXER_KEYWORDS_H */
