/**
 * Validate - validate all semantics of a program, as well as building the AST.
 */

#ifndef SEMANTICS_VALIDATE_H
#define SEMANTICS_VALIDATE_H

#include <api/compilation-data.h>

/**
 * Returns:
 * 0 - all OK
 * 1 - semantic error
 * 2 - internal error
 */
int yfs_validate(struct yf_file_compilation_data *);

#endif /* SEMANTICS_VALIDATE_H */
