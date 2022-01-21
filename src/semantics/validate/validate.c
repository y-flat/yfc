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
    yfv_add_type(fdata, type);
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

    /* We're considering bool to be one bit for conversion purposes. */
    add_type(fdata, "bool",       1,  YFS_INT  );

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
    struct yf_file_compilation_data * fdata,
    struct yfs_type * for_bstmt1,
    int * for_bstmt2
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
        return validate_bstmt(csub, asub, pdata, fdata, for_bstmt1, for_bstmt2);
    case YFCS_RET:
        return validate_return(csub, asub, pdata, fdata, for_bstmt1, for_bstmt2);
    case YFCS_IF:
        return validate_if(csub, asub, pdata, fdata, for_bstmt1, for_bstmt2);
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
        if (validate_node(cnode, anode, pdata, fdata, NULL, NULL)) {
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

int validate_return(
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata,
    struct yfs_type * type, int * returns
) {

    struct yfcs_return * c = &cin->ret;
    struct yfa_return  * a = &ain->ret;

    ain->type = YFA_RETURN;
    a->expr = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->expr)
        return 2;

    if (c->expr) {
        if (validate_expr(c->expr, a->expr, pdata, fdata)) {
            fdata->error = 1;
            return 1;
        }
    } else {
        a->expr = NULL;
    }

    if ( (type->primitive.size != 0) ?
            yfs_output_diagnostics(
                (a->expr != NULL)
                    ? yfse_get_expr_type(&a->expr->expr, fdata)
                    : yfv_get_type_s(fdata, "void"),
                type,
                fdata,
                cin->lineno
            )
        :   (a->expr != NULL)
    ) {
        fdata->error = 1;
        return 1;
    }

    *returns = 1;

    return 0;

}
