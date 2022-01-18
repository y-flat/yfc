#include <semantics/validate/validate-internal.h>

#include <semantics/types.h>

/**
 * Takes an input of a vardecl node.
 */
int validate_vardecl(
    struct yf_parse_node * cin,
    struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {

    struct yf_sym * entry;

    struct yfcs_vardecl * c = &cin->vardecl;
    struct yfa_vardecl  * a = &ain->vardecl;
    ain->type = YFA_VARDECL;

    int ssym;
    bool global = (current_scope == &fdata->symtab);

    /* Make sure it isn't declared twice */
    /* A variable is redeclared in the current scope. Whoops! */
    /* Only report this for non-global decls, those have been caught already
    in symtab building. */
    if ( (
        ssym = find_symbol(&entry, current_scope, c->name.name)
    ) != -1 && !global) {
        if (ssym == 0) {
            YF_PRINT_ERROR(
                "Duplicate declaration of symbol '%s'"
                ", lines %d and %d",
                c->name.name,
                entry->line,
                cin->lineno
            );
            return 1;
        } else {
            /* Just a warning about shadowing. */
            YF_PRINT_WARNING(
                "Global symbol '%s' (line %d) "
                "shadowed by local symbol (line %d)",
                c->name.name,
                entry->line,
                cin->lineno
            );
        }
    }

    /* Construct abstract instance, ONLY if non-global */

    a->name = yf_malloc(sizeof (struct yf_sym));
    if (!a->name)
        return 2;

    a->name->type = YFS_VAR;

    /* Add to symbol table UNLESS it is global scope. */
    /* The global scope symtab is already set up. */
    if (!global) {
        a->name->var.name = c->name.name;
        yfh_set(current_scope->table, c->name.name, a->name);
    } else {
        /* Free the name, since it was only needed for type checking. */
        free(a->name);
        /* If it's global, set "name" to point to the global symbol. */
        a->name = entry;
    }

    /* Verify type */
    /* We have to check that the type is valid here, because the type table
    doesn't exist during the symtab-building phase. */
    if ( (a->name->var.dtype = yfh_get(fdata->types.table, c->type.databuf)) == NULL) {
        YF_PRINT_ERROR(
            "Unknown type '%s' in declaration of '%s' (line %d)",
            c->type.databuf,
            c->name.name,
            cin->lineno
        );
        free(a->name);
        return 1;
    }

    /* Variables can't have type "void" */
    if (
        a->name->var.dtype->kind == YFST_PRIMITIVE
        && a->name->var.dtype->primitive.size == 0
    ) {
        YF_PRINT_ERROR(
            "Variable '%s' has type 'void' (line %d)",
            c->name.name,
            cin->lineno
        );
        free(a->name);
        return 1;
    }
    
    a->name->line = cin->lineno;

    if (c->expr) {
        a->expr = yf_malloc(sizeof (struct yf_ast_node));
        if (!a->expr)
            return 2;
        a->expr->type = YFA_EXPR;
        if (validate_expr(c->expr, a->expr, pdata, fdata))
            return 1;
    } else {
        a->expr = NULL;
    }

    /* Check that the types are compatible. */
    if (a->expr) {
        if (yfs_output_diagnostics(
            yfse_get_expr_type(&a->expr->expr, fdata),
            a->name->var.dtype,
            fdata,
            cin->lineno
        )) {
            return 1;
        }
    }
    
    return 0;

}
