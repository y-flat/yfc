/**
 * Functions that don't directly concern the validation - just used to remove
 * the utility functions from the actual validation logic.
 */

#ifndef SEMANTICS_VALIDATE_UTILS_H
#define SEMANTICS_VALIDATE_UTILS_H

#include <api/compilation-data.h>
#include <api/operator.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

/**
 * Internal - the innermost scope we have open. TODO - un-global this.
 */
extern struct yfs_symtab * current_scope;

/**
 * Search for a symbol with the given name. Return "depth" - innermost scope is
 * 0, the next-enclosing is 1, etc. If not found, -1.
 */
int find_symbol(
    struct yf_sym ** sym, struct yfs_symtab * symtab,
    char * name
);

#endif /* SEMANTICS_VALIDATE_UTILS_H */
