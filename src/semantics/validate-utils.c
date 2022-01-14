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

int enter_scope(struct yfs_symtab ** stuff) {

    struct yfs_symtab * old_symtab, * new_symtab;
    old_symtab = current_scope;

    new_symtab = yf_malloc(sizeof (struct yfs_symtab));
    if (!new_symtab) {
        return 1;
    }
    new_symtab->table = yfh_new();
    if (!new_symtab->table) {
        free(new_symtab);
        return 1;
    }

    new_symtab->parent = old_symtab;
    current_scope = new_symtab;

    if (stuff)
        *stuff = new_symtab;
    
    return 0;

}

void exit_scope(void) {
    /* Don't free scope - used later during codegen */
    current_scope = current_scope->parent;
}
