/**
 * The only public parser function, which parses an entire file. More may be
 * made visible as needed.
 */

#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include <api/parse-tree.h>

/**
 * Parse data from file into tree.
 * Returns: error code, or 0 if successful.
 */
int yf_parse_file(const char * filename, struct yf_parse_tree * tree);

#endif /* PARSER_PARSER_H */
