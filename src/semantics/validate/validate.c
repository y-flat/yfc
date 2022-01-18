#include "validate.h"

#include <semantics/types.h>
#include <semantics/validate/validate-internal.h>

void add_type(
    struct yf_file_compilation_data * fdata,
    char * name, int size, enum yfpt_format fmt) {
    struct yfs_type * type = yf_malloc(sizeof (struct yfs_type));
    type->primitive.size = size;
    type->kind = YFST_PRIMITIVE;
    type->primitive.type = fmt;
    type->name = name;
    yfh_set(fdata->types.table, name, type);
}

void yfv_add_builtin_types(struct yf_file_compilation_data * fdata) {

    /* All types are signed for now - unsigned types are not yet supported. */

    /* "standard" types. */
    add_type(fdata, "char",        8, YFS_INT  );
    add_type(fdata, "short",      16, YFS_INT  );
    add_type(fdata, "int",        32, YFS_INT  );
    add_type(fdata, "long",       64, YFS_INT  );
    add_type(fdata, "void",        0, YFS_NONE );
    add_type(fdata, "float",      32, YFS_FLOAT);
    add_type(fdata, "double",     64, YFS_FLOAT);

    /* Convenience types. */
    add_type(fdata, "i16",        16, YFS_INT  );
    add_type(fdata, "i32",        32, YFS_INT  );
    add_type(fdata, "i64",        64, YFS_INT  );
    add_type(fdata, "f16",        16, YFS_FLOAT);
    add_type(fdata, "f32",        32, YFS_FLOAT);
    add_type(fdata, "f64",        64, YFS_FLOAT);

}

int yfs_validate(
    struct yf_file_compilation_data * fdata,
    struct yf_project_compilation_data * pdata
) {
    fdata->types.table = yfh_new();
    yfv_add_builtin_types(fdata);
    return validate_program(
        &fdata->parse_tree, &fdata->ast_tree, pdata, fdata
    );
}

int validate_node(
    struct yf_parse_node * csub, struct yf_ast_node * asub,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {
    switch (csub->type) {
    case YFCS_EXPR:
        return validate_expr(csub, asub, pdata, fdata);
    case YFCS_VARDECL:
        return validate_vardecl(csub, asub, pdata, fdata);
    case YFCS_FUNCDECL:
        return validate_funcdecl(csub, asub, pdata, fdata);
    case YFCS_PROGRAM:
        return validate_program(csub, asub, pdata, fdata);
    case YFCS_BSTMT:
        return validate_bstmt(csub, asub, pdata, fdata);
    default:
        YF_PRINT_ERROR("internal error: unknown CST node type");
        return 1;
    } 
}

int validate_program(
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {

    struct yf_parse_node * cnode;
    struct yf_ast_node * anode;
    struct yfcs_program * cprog;
    struct yfa_program * aprog;
    int err = 0;

    cprog = &cin->program;
    aprog = &ain->program;
    ain->type = YFA_PROGRAM;

    yf_list_init(&aprog->decls);
    yf_list_reset(&cprog->decls);

    /* Initialize the scope */
    current_scope = &fdata->symtab;
    
    /* Iterate through all decls, construct abstract instances of them, and move
    them into the abstract list. */
    for (;;) {

        /* Get element */
        if (yf_list_get(&cprog->decls, (void **) &cnode) == -1) break;
        if (!cnode) break;
        
        /* Construct abstract instance */
        anode = yf_malloc(sizeof (struct yf_ast_node));
        if (!anode)
            return 2;

        /* Validate */
        if (validate_node(cnode, anode, pdata, fdata)) {
            yf_free(anode);
            fdata->error = 1;
            err = 1;
            /* No return, keep going to find more errors. */
        }

        /* Move to abstract list */
        yf_list_add(&aprog->decls, anode);

        /* Keep going */
        yf_list_next(&cprog->decls);

    }

    return err;

}

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
