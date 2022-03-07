/**
 * Defines utilities to build a symbol table of a file.
 */

#ifndef SEMANTICS_SYMTAB_H
#define SEMANTICS_SYMTAB_H

#include <api/compilation-data.h>

/**
 * 0 - success, 1 - error
 */
int yfs_build_symtab(struct yf_compile_analyse_job *);

#endif /* SEMANTICS_SYMTAB_H */
