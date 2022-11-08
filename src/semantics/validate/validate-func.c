#include <semantics/validate/validate-internal.h>

int validate_funcdecl(
    struct yfv_validator * validator,
    struct yf_parse_node * cin, struct yf_ast_node * ain
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
        ssym = find_symbol(validator, &a->name, &c->name)
    ) == -1) {
        /* Uh oh ... */
        YF_PRINT_ERROR("internal error: symbol not found");
        return 2;
    }

    struct yfs_type * ty;

    if ((ty = yfv_get_type_t(
        validator->udata, c->ret
    )) == NULL) {
        YF_PRINT_ERROR(
            "%s %d:%d: return type not found",
            cin->loc.file,
            cin->loc.line,
            cin->loc.column
        );
        return 2;
    }

    a->name->fn.rtype = *ty;

    /* Now, validate the argument list. */
    /* Also, open a new scope for arguments. */
    enter_scope(validator, &a->param_scope);

    /* Add the arguments to the scope. */
    yf_list_init(&a->params);
    YF_LIST_FOREACH(c->params, cv) {
        av = yf_malloc(sizeof (struct yf_ast_node));
        if (!av)
            return 2;
        if (validate_vardecl(validator, cv, av)) {
            yf_free(av);
            validator->error = 1;
            return 1;
        }
        yf_list_add(&a->params, av);
    }

    /* Validate the return type. */
    if ((a->ret = yfv_get_type_t(validator->udata, c->ret)) == NULL) {
        YF_PRINT_ERROR(
            "%s %d:%d: Unknown return type '%s' of function '%s'",
            cin->loc.file,
            cin->loc.line,
            cin->loc.column,
            c->ret.databuf,
            c->name.name
        );
        return 1;
    }

    a->body = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->body)
        return 2;

    /* Now, validate the body. */
    if (validate_bstmt(validator, c->body, a->body, a->ret, &returns)) {
        yf_free(a->body);
        a->body = NULL;
        return 1;
    }

    /* Close the scope. */
    exit_scope(validator);

    if (returns == 0 && a->ret->primitive.size != 0) {
        YF_PRINT_ERROR(
            "%s %d:%d: Function '%s' does not always return a value",
            cin->loc.file,
            cin->loc.line,
            cin->loc.column,
            c->name.name
        );
        return 1;
    }

    return 0;

}

int validate_bstmt(
    struct yfv_validator * validator,
    struct yf_parse_node * cin, struct yf_ast_node * ain,
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
    enter_scope(validator, &a->symtab);

    /* Validate each statement */
    yf_list_init(&a->stmts);

    *returns = 0;

    YF_LIST_FOREACH(c->stmts, csub) {

        /* If this comes after a return, none of it will be executed. */
        if (*returns && !ret_warning_reported) {
            ret_warning_reported = 1;
            YF_PRINT_WARNING(
                "File %s: code on line %d until the end of the current "
                "block will never execute",
                csub->loc.file,
                csub->loc.line
            );
        }
        
        /* Construct abstract instance */
        asub = yf_malloc(sizeof (struct yf_ast_node));
        if (!asub)
            return 2;

        /* Validate */
        if (validate_node(validator, csub, asub, type, returns)) {
            yf_free(asub);
            validator->error = 1;
            err = 1;
        } else {

            /* Move to abstract list */
            yf_list_add(&a->stmts, asub);

            /* Probably redundant */
            if (asub->type == YFA_RETURN) {
                *returns = 1;
            }
            
        }

    }

    exit_scope(validator);

    return err;

}
