#include <semantics/validate/validate-internal.h>

#include <ctype.h>
#include <string.h>

#include <api/abstract-tree.h>
#include <semantics/types.h>

static int validate_expr_e(struct yfcs_expr * c, struct yfa_expr * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata, int lineno
);

/**
 * Validate various kinds of expressions - funccalls, values, and binaries.
 */

static int validate_value(
    struct yfcs_value * c,
    struct yfa_value  * a,
    int lineno
) {

    char * intparse;
    char dig;

    /* If an identifier, make sure it actually exists. */
    if (c->type == YFCS_IDENT) {
        if (find_symbol(
            &a->as.identifier,
            current_scope,
            c->identifier.name
        ) == -1) {
            YF_PRINT_ERROR(
                "Unknown identifier '%s' (line %d)",
                c->identifier.name,
                lineno
            );
            return 1;
        }
        a->type = YFA_IDENT;
    } else {

        a->type = YFA_LITERAL;

        if (isdigit(c->literal.value[0])) {

            /* Parse integer literal */
            a->as.literal.type = YFAL_NUM;
            a->as.literal.val = 0;

            for (intparse = c->literal.value; *intparse; ++intparse) {
                dig = *intparse - '0';
                if (dig < 0 || dig > 9) {
                    YF_PRINT_ERROR(
                        "Invalid literal '%s' (line %d), "
                        "found invalid character '%c' in int literal",
                        c->literal.value, lineno, *intparse
                    );
                    return 1;
                }
                a->as.literal.val *= 10;
                a->as.literal.val += dig;
            }

            return 0;

        }

        /* Check for 'true' or 'false' */
        if (strcmp(c->literal.value, "true") == 0) {
            a->as.literal.type = YFAL_BOOL;
            a->as.literal.val = 1;
            return 0;
        }
        if (strcmp(c->literal.value, "false") == 0) {
            a->as.literal.type = YFAL_BOOL;
            a->as.literal.val = 0;
            return 0;
        }

    }

    return 0;

}

static int validate_binary(
    struct yfcs_binary * c, struct yfa_binary * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata, int lineno
) {
    /* First, if it's an assignment, the left side is a variable. */
    if (yfo_is_assign(c->op)) {
        if (c->left->type != YFCS_VALUE) {
            YF_PRINT_ERROR(
                "Left side of assignment must not be compound (line %d)",
                lineno
            );
            return 1;
        }
        if (c->left->expr.value.type != YFCS_IDENT) {
            YF_PRINT_ERROR(
                "Left side of assignment must be an identifier (line %d)",
                lineno
            );
            return 1;
        }
    }

    a->op = c->op;

    a->left = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->left)
        return 2;
    if (validate_expr_e(
        &c->left->expr, a->left, pdata, fdata, lineno
    ))
        return 1;

    a->right = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->right)
        return 2;
    if (validate_expr_e(
        &c->right->expr, a->right, pdata, fdata, lineno
    ))
        return 1;

    /* Check that the types are compatible. */
    if (yfs_output_diagnostics(
        yfse_get_expr_type(a->left, fdata),
        yfse_get_expr_type(a->right, fdata),
        fdata,
        lineno
    )) {
        return 1;
    }

    return 0;

}


static int validate_funccall(
    struct yfcs_funccall * c, struct yfa_funccall * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata, int lineno
) {


    struct yf_parse_node * carg;
    struct yf_ast_node   * aarg;
    struct yfsn_param    * param;
    struct yfs_type      * paramtype;

    int lgres;

    /* Make sure the function exists. */
    if (find_symbol(
        &a->name, current_scope, c->name.name
    ) == -1) {
        YF_PRINT_ERROR(
            "Unknown function '%s' (line %d)",
            c->name.name,
            c->name.lineno
        );
        return 1;
    }

    /* Make sure the function is actually a function. */
    if (a->name->type != YFS_FN) {
        YF_PRINT_ERROR(
            "Identifier '%s' is not a function (line %d)",
            c->name.name,
            c->name.lineno
        );
        return 1;
    }

    /* Go through the arguments and add them to the list, while making sure
        * the types are compatible for each one and the number of arguments
        * matches.
        */
    yf_list_init(&a->args);
    yf_list_reset(&a->name->fn.params);
    yf_list_reset(&c->args);
    for (;;) {

        aarg = yf_malloc(sizeof (struct yf_ast_node));
        if (!aarg)
            return 2;

        if (
            yf_list_get(&a->name->fn.params, (void **) &param) !=
            (lgres = yf_list_get(&c->args, (void **) &carg))
        ) {
            YF_PRINT_ERROR(
                "line %d: too %s arguments in function call",
                lineno,
                lgres ? "few" : "many"
            );
            return 1;
        }
        if (lgres == -1) {
            break;
        }

        if (validate_expr(
            carg, aarg, pdata, fdata
        )) {
            return 1;
        }

        if ( (paramtype = yfv_get_type_s(fdata, param->type)) == NULL) {
            YF_PRINT_ERROR(
                "line %d: Uncaught type error: unknown type '%s'",
                lineno,
                param->type
            );
            return 1;
        }

        if (yfs_output_diagnostics(
            yfse_get_expr_type(&aarg->expr, fdata),
            paramtype,
            fdata,
            lineno
        )) {
            return 1;
        }

        yf_list_add(&a->args, aarg);
        yf_list_next(&c->args);
        yf_list_next(&a->name->fn.params);

    }

    return 0;

}

/**
 * Takes in parameters of expr, rather than node, types.
 */
static int validate_expr_e(struct yfcs_expr * c, struct yfa_expr * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata, int lineno
) {
    
    /* If this is unary - (just a value), ... */
    switch (c->type) {

    case YFCS_VALUE:
        a->type = YFA_VALUE;
        return validate_value(&c->value, &a->as.value, lineno);

    case YFCS_BINARY:
        a->type = YFA_BINARY;
        return validate_binary(&c->binary, &a->as.binary, pdata, fdata, lineno);

    case YFCS_FUNCCALL:
        a->type = YFA_FUNCCALL;
        return validate_funccall(&c->call, &a->as.call, pdata, fdata, lineno);
    }

    return 0;

}

int validate_expr(struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {
    ain->type = YFA_EXPR;
    return validate_expr_e(&cin->expr, &ain->expr, pdata, fdata, cin->lineno);
}
