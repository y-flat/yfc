/**
 * Validate - validate all semantics of a program, as well as building the AST.
 */

#ifndef SEMANTICS_VALIDATE_H
#define SEMANTICS_VALIDATE_H

#include <api/compilation-data.h>

int yfs_validate(struct yf_file_compilation_data *);

#endif /* SEMANTICS_VALIDATE_H */
