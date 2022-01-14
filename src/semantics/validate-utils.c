#include "validate-utils.h"

struct yfs_symtab * current_scope;

int find_symbol(
    struct yf_sym ** sym, struct yfs_symtab * symtab,
    char * name
) {
    int depth = 0;
    while (symtab != NULL) {
        if ( (*sym = yfh_get(symtab->table, name)) != NULL) {
            return depth;
        }
        depth++;
        symtab = symtab->parent;
    }
    return -1;
}
