/**
 * Loading a symbol table from a cached library symbol file.
 */

#ifndef SEMANTICS_LIBSYMS_H
#define SEMANTICS_LIBSYMS_H

#include <util/hashmap.h>

/**
 * Returns:
 * 0 - successful
 * 1 - could not find file
 * 2 - symbol table in invalid format
 * 3 - could not put into symtab
 */
int yfs_load_symtab(const char * module_name, struct yf_hashmap *);

#endif /* SEMANTICS_LIBSYMS_H */
