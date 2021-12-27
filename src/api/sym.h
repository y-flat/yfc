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

struct yfs_type {

    /* Nothing much yet */
    char * name;

};

struct yfs_var {

    char * name;
    struct yfs_type dtype; /* "declared type" */    

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

    struct yf_hashmap table;

};

#endif /* API_SYM_H */
