/**
 * Dump a concrete syntax tree to a file for debugging purposes.
 */

#ifndef API_CST_DUMP_H
#define API_CST_DUMP_H

#include <stdio.h>

#include <api/concrete-tree.h>

void yf_dump_cst(struct yf_parse_node * root, FILE *out);

#endif /* API_CST_DUMP_H */
