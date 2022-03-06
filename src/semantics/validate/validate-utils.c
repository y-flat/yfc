#include <semantics/validate/validate-internal.h>

/**
 * Search all scopes, starting with the innermost one and heading out.
 * EXAMPLE:
 * x: int = 3; ~~ This scope last ~~
 * foo() {
 *  x: int = 4; ~~ This scope second ~~
 *  if (true) {
 *      x: int = 5; ~~ This scope is searched first ~~
 *  }
 * }
 */
int find_symbol(
    struct yfv_validator * validator,
    struct yf_sym ** sym,
    char * name
) {
    int depth = 0;
    struct yfs_symtab * symtab = validator->current_scope;
    while (symtab != NULL) {
        if ( (*sym = yfh_get(symtab->table, name)) != NULL) {
            return depth;
        }
        depth++;
        symtab = symtab->parent;
    }
    return -1;
}

int enter_scope(struct yfv_validator * v, struct yfs_symtab ** stuff) {

    struct yfs_symtab * old_symtab, * new_symtab;
    old_symtab = v->current_scope;

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
    v->current_scope = new_symtab;

    if (stuff)
        *stuff = new_symtab;
    
    return 0;

}

void exit_scope(struct yfv_validator * v) {
    /* Don't free scope - used later during codegen */
    v->current_scope = v->current_scope->parent;
}

int yfv_add_type(
    struct yf_compile_analyse_job * udata,
    struct yfs_type * type
) {
    /**
     * Set a value in the hashmap.
     */
    return yfh_set(udata->types.table, type->name, type);
}

struct yfs_type * yfv_get_type_t(
    struct yf_compile_analyse_job * udata,
    struct yfcs_type type
) {
    /**
     * Get a value from the hashmap.
     */
    return yfv_get_type_s(udata, type.databuf);
}

struct yfs_type * yfv_get_type_s(
    struct yf_compile_analyse_job * udata,
    char * typestr
) {
    /**
     * Get a value from the hashmap.
     */
    return yfh_get(udata->types.table, typestr);
}
