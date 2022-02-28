#include <semantics/types.h>
#include <semantics/validate/validate-internal.h>

int validate_if(
    struct yfv_validator * validator,
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yfs_type * type,
    int * returns
) {
    
    struct yfcs_if * c = &cin->ifstmt;
    struct yfa_if  * a = &ain->ifstmt;
    int if_always_returns = 0, else_always_returns = 0;
    struct yfs_type * t;
    
    ain->type = YFA_IF;

    a->cond = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->cond)
        return 2;
    a->code = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->code)
        return 2;

    if (validate_expr(validator, c->cond, a->cond)) {
        validator->fdata->error = 1;
        return 1;
    }
    
    if ( (t = yfse_get_expr_type(
        &a->cond->expr, validator->fdata
    )) != yfv_get_type_s(validator->fdata, "bool")) {
        YF_PRINT_ERROR(
            "%s %d: %d: if condition must be of type bool, was %s",
            cin->loc.file, cin->loc.line, cin->loc.column,
            t->name
        );
        validator->fdata->error = 1;
        return 1;
    }

    if (validate_node(
        validator,
        c->code, a->code,
        type, &if_always_returns
    )) {
        validator->fdata->error = 1;
        yf_free(a->code);
        a->code = NULL;
        return 1;
    }

    if (c->elsebranch) {
        a->elsebranch = yf_malloc(sizeof (struct yf_ast_node));
        if (!a->elsebranch)
            return 2;
        if (validate_node(
            validator,
            c->elsebranch, a->elsebranch,
            type, &else_always_returns
        )) {
            validator->fdata->error = 1;
            return 1;
        }
    } else {
        a->elsebranch = NULL;
    }

    /* If both "if" and "else" always return, then the entire if statement
     * always returns.
     */
    if (if_always_returns && else_always_returns)
        *returns = 1;
    else
        *returns = 0;

    return 0;

}
