#include <semantics/validate/validate-internal.h>

#include <string.h>

#include <util/yfc-out.h>

/**
 * Look up a name in a given scope.
 * EXAMPLE:
 * x: int = 3; ~~ This scope last ~~
 * foo() {
 *  x: int = 4; ~~ This scope second ~~
 *  if (true) {
 *      x: int = 5; ~~ This scope is searched first ~~
 *  }
 * }
 */
static int find_symbol_from_scope(
    struct yfs_symtab * symtab,
    struct yf_sym ** sym,
    char * name
) {
    int depth = 0;
    while (symtab != NULL) {
        if (yfh_get(&symtab->table, name, (void **)sym) == 0) {
            return depth;
        }
        depth++;
        symtab = symtab->parent;
    }
    return -1;
}

/**
 * If the identifier has no prefix, search the current file. Otherwise, look up
 * the loaded file.
 */
int find_symbol(
    struct yfv_validator * validator,
    struct yf_sym ** sym,
    struct yfcs_identifier * name
) {
    if (!strcmp(name->filepath, "")) {
        return find_symbol_from_scope(
            validator->current_scope,
            sym,
            name->name
        );
    } else {
        struct yfs_symtab * symtab;
        if (yfh_get(
            &validator->pdata->symtables, name->filepath, (void **)&symtab
        )) {
            YF_PRINT_ERROR("Could not find module: %s", name->filepath);
            return -1;
        }
        return find_symbol_from_scope(
            symtab,
            sym,
            name->name
        );
    }
}

int enter_scope(struct yfv_validator * v, struct yfs_symtab ** stuff) {

    struct yfs_symtab * old_symtab, * new_symtab;
    old_symtab = v->current_scope;

    new_symtab = yf_malloc(sizeof (struct yfs_symtab));
    if (!new_symtab) {
        return 1;
    }
    yfh_init(&new_symtab->table);
    if (!new_symtab->table.buckets) {
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
    return yfh_set(&udata->types.table, type->name, type);
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
    struct yfs_type * result = NULL;
    yfh_get(&udata->types.table, typestr, (void **)&result);
    return result;
}
