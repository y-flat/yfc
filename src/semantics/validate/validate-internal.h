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

/**
 * Create a new scope - return 0 on success, 1 on failure (memory error).
 * The root of the created symtab is set to the current scope, and the current
 * scope is also set to the new scope.
 */
int enter_scope(struct yfs_symtab ** stuff);

/**
 * Exit a scope.
 */
void exit_scope(void);

/* Any sort of "typical" transfer operation - takes a current file for local
 * decls, the project for all decls, and the two nodes to edit. */

/* I would use a typedef, but then the forwards would conflict. */
#define VDECL(name) int name( \
    struct yf_parse_node *, \
    struct yf_ast_node *, \
    struct yf_project_compilation_data *, \
    struct yf_file_compilation_data *\
)

VDECL(validate_program);
VDECL(validate_funcdecl);
VDECL(validate_expr);
VDECL(validate_vardecl);

int validate_node(
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata,
    struct yfs_type * type, int * returns
);

/**
 * Different - a type is passed in, so the expr can be checked.
 */
int validate_return(
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata,
    struct yfs_type * type
);
/**
 * For block statements, we need to reason about what they return.
 * "returns" is stuffed with 1 if it *always* returns, and 0 otherwise.
 */
int validate_bstmt(
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata,
    struct yfs_type * type,
    int * returns
);

/**
 * Add a type to a file's type table.
 */
int yfv_add_type(
    struct yf_file_compilation_data * fdata,
    struct yfs_type * type
);

/**
 * Get a type from a file's type table, given a concrete type.
 */
struct yfs_type * yfv_get_type_t(
    struct yf_file_compilation_data * fdata,
    struct yfcs_type type
);

/**
 * Get a type from a file's type table, given a string.
 */
struct yfs_type * yfv_get_type_s(
    struct yf_file_compilation_data * fdata,
    char * typestr
);

#endif /* SEMANTICS_VALIDATE_UTILS_H */
