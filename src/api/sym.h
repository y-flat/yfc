/**
 * A globally visible symbol. This should represent any top-level declaration in
 * a file (except for imports.)
 * 
 * This file also defines all accompanying definitions, such as a symbol table,
 * a type, etc.
 */

#ifndef API_SYM_H
#define API_SYM_H

#include <util/hashmap.h>

struct yfs_primitive_type {
    int size; /* In bits */
};

/* Has no name - types are stored in a hashmap. NO nesting needed - all types
are globally visible. */
struct yfs_type {

    union {
        struct yfs_primitive_type primitive;
    };

    enum {
        YFST_PRIMITIVE,
    } kind;

};

struct yfs_var {

    char * name;
    struct yfs_type * dtype; /* "declared type" */    

};

struct yf_sym {

    enum {
        YFS_VAR,
    } type;

    union {
        struct yfs_var var;
    };

    char * file; /* Source file */
    int line; /* Line number of declaration */

};

struct yfs_symtab {

    struct yf_hashmap * table;

    /* Also, a pointer to the "parent" symtab. For top-level decls, this is the
     * file symtab, and for file symtabs this is NULL. */
    struct yfs_symtab * parent;

};

struct yfs_type_table {

    struct yf_hashmap * table;

};

#endif /* API_SYM_H */
