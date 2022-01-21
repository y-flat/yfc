/**
 * Generating type names in a C-readable manner.
 */

#ifndef YF_GEN_TYPEGEN_H
#define YF_GEN_TYPEGEN_H

#include <api/sym.h>

/**
 * Stuff a C form of a type into a buffer.
 *  0 - success
 *  1 - overflow
 * -1 - other error
 */
int yfg_ctype(int len, char * buf, struct yfs_type * type);

#endif /* YF_GEN_TYPEGEN_H */
