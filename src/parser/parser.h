/**
 * The only public parser function, which parses an entire file. More may be
 * made visible as needed.
 */

#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include <api/concrete-tree.h>
#include <lexer/lexer.h>

/**
 * Parse data from lexer into tree.
 * Returns: error code, or 0 if successful.
 */
int yf_parse(struct yf_lexer * lexer, struct yf_parse_node * tree);

#endif /* PARSER_PARSER_H */
