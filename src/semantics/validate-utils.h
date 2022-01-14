/**
 * Functions that don't directly concern the validation - just used to remove
 * the utility functions from the actual validation logic.
 */

#ifndef SEMANTICS_VALIDATE_UTILS_H
#define SEMANTICS_VALIDATE_UTILS_H

/**
 * Internal - the innermost scope we have open. TODO - un-global this.
 */
extern struct yfs_symtab * current_scope;

#include <api/compilation-data.h>
#include <api/operator.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

#endif /* SEMANTICS_VALIDATE_UTILS_H */
