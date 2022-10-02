/**
 * A globally visible symbol. This should represent any top-level declaration in
 * a file (except for imports.)
 * 
 * This file also defines all accompanying definitions, such as a symbol table,
 * a type, etc.
 */

#ifndef API_SYM_H
#define API_SYM_H

#include <api/loc.h>
#include <util/list.h>
#include <util/hashmap.h>

/**
 * What 'kind' of number does a primitive type represent?
 */
enum yfpt_format {
    YFS_F_INT, /* NOT just "int", but a whole number. */
    YFS_F_FLOAT, /* ANY number with a decimal part. */
    YFS_F_NONE,
};

struct yfs_primitive_type {
    int size; /* In bits */
    enum yfpt_format type;
};

/* Has names even though types are stored in a hashmap, in case errors are to be
reported. NO nesting needed - all types are globally visible. */
struct yfs_type {

    union {
        struct yfs_primitive_type primitive;
    };

    enum {
        YFS_T_PRIMITIVE,
    } kind;

    char * name; /* Name of the type */

};

/**
 * Free up a type.
 */
void yfs_cleanup_type(struct yfs_type * type);

struct yfs_var {

    char * name;
    struct yfs_type * dtype; /* "declared type" */    

};

/**
 * This is a parameter for a function. The reason this isn't a vardecl is that
 * a symbol table needs variable types, but when a symbol table is being built
 * the types that exist are not yet known.
 */
struct yfsn_param {
    char * name, * type;
};

struct yfs_fn {

    char * name;
    struct yfs_type * rtype; /* "return type" */
    struct yf_list    params; /* list of param */

};

struct yf_sym {

    enum {
        YFS_VAR,
        YFS_FN,
    } type;

    union {
        struct yfs_var var;
        struct yfs_fn  fn;
    };

    struct yf_location loc;

};

void yfs_cleanup_sym(struct yf_sym * sym);

struct yfs_symtab {

    struct yf_hashmap table;

    /* Also, a pointer to the "parent" symtab. For top-level decls, this is the
     * file symtab, and for file symtabs this is NULL. */
    struct yfs_symtab * parent;

};

struct yfs_type_table {

    struct yf_hashmap table;

};

#endif /* API_SYM_H */
