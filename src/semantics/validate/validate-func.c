#include <semantics/validate/validate-internal.h>

int validate_funcdecl(
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {
    
    struct yfcs_funcdecl  * c = &cin->funcdecl;
    struct yfa_funcdecl   * a = &ain->funcdecl;
    struct yf_parse_node  * cv;
    struct yf_ast_node    * av;

    int ssym;
    int returns;

    ain->type = YFA_FUNCDECL;

    /* Functions are only global. */
    if ( (
        ssym = find_symbol(&a->name, current_scope, c->name.name)
    ) == -1) {
        /* Uh oh ... */
        YF_PRINT_ERROR("internal error: symbol not found");
        return 2;
    }

    if ((a->name->fn.rtype = yfv_get_type_t(
        fdata, c->ret
    )) == NULL) {
        YF_PRINT_ERROR("error: %d: return type not found", cin->lineno);
        return 2;
    }

    /* Now, validate the argument list. */
    /* Also, open a new scope for arguments. */
    enter_scope(NULL);

    /* Add the arguments to the scope. */
    yf_list_reset(&c->params);
    yf_list_init(&a->params);
    for (;;) {
        if (yf_list_get(&c->params, (void**) &cv) == -1)
            break;
        av = yf_malloc(sizeof (struct yf_ast_node));
        if (!av)
            return 2;
        if (validate_vardecl(cv, av, pdata, fdata))
            return 1;
        yf_list_add(&a->params, av);
        yf_list_next(&c->params);
    }

    /* Validate the return type. */
    if ((a->ret = yfv_get_type_t(fdata, c->ret)) == NULL) {
        YF_PRINT_ERROR(
            "Unknown return type '%s' of function '%s' (line %d)",
            c->ret.databuf,
            c->name.name,
            cin->lineno
        );
        return 1;
    }

    a->body = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->body)
        return 2;

    /* Now, validate the body. */
    if (validate_bstmt(c->body, a->body, pdata, fdata, a->ret, &returns))
        return 1;

    /* Close the scope. */
    exit_scope();

    if (returns == 0 && a->ret->primitive.size != 0) {
        YF_PRINT_ERROR(
            "Function '%s' does not always return a value (line %d)",
            c->name.name,
            cin->lineno
        );
        return 1;
    }

    return 0;

}

int validate_bstmt(struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata,
    struct yfs_type * type,
    int * returns
) {

    struct yfcs_bstmt * c = &cin->bstmt;
    struct yfa_bstmt * a = &ain->bstmt;

    struct yf_parse_node * csub;
    struct yf_ast_node * asub;
    int err = 0;
    int ret_warning_reported = 0;

    ain->type = YFA_BSTMT;
    
    /* Create a symbol table for this scope */
    enter_scope(&a->symtab);

    /* Validate each statement */
    yf_list_reset(&c->stmts);
    yf_list_init(&a->stmts);

    *returns = 0;

    for (;;) {

        /* Get element */
        if (yf_list_get(&c->stmts, (void **) &csub) == -1) break;
        if (!csub) break;

        /* If this comes after a return, none of it will be executed. */
        if (*returns && !ret_warning_reported) {
            ret_warning_reported = 1;
            YF_PRINT_WARNING(
                "Code on line %d until the end of the current "
                "block will never execute",
                csub->lineno
            );
        }
        
        /* Construct abstract instance */
        asub = yf_malloc(sizeof (struct yf_ast_node));
        if (!asub)
            return 2;

        /* Validate */
        if (validate_node(csub, asub, pdata, fdata, type, returns)) {
            yf_free(asub);
            fdata->error = 1;
            err = 1;
        } else {
            /* Move to abstract list */
            yf_list_add(&a->stmts, asub);
        }

        /* Probably redundant */
        if (asub->type == YFA_RETURN) {
            *returns = 1;
        }

        /* Keep going */
        yf_list_next(&c->stmts);
        
    }

    exit_scope();

    return err;

}
