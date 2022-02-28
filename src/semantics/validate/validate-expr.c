#include <semantics/validate/validate-internal.h>

#include <ctype.h>
#include <string.h>

#include <api/abstract-tree.h>
#include <semantics/types.h>

static int validate_expr_e(
    struct yfv_validator * validator,
    struct yfcs_expr * c, struct yfa_expr * a, struct yf_location * loc
);

/**
 * Validate various kinds of expressions - funccalls, values, and binaries.
 */

static int validate_value(
    struct yfv_validator * validator,
    struct yfcs_value * c,
    struct yfa_value  * a,
    struct yf_location * loc
) {

    char * intparse;
    char dig;

    /* If an identifier, make sure it actually exists. */
    if (c->type == YFCS_IDENT) {
        if (find_symbol(
            validator,
            &a->as.identifier,
            c->identifier.name
        ) == -1) {
            YF_PRINT_ERROR(
                "%s %d:%d: Unknown identifier '%s'",
                loc->file,
                loc->line,
                loc->column,
                c->identifier.name
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
                        "%s %d:%d: Invalid literal '%s', "
                        "found invalid character '%c' in int literal",
                        loc->file, loc->line, loc->column,
                        c->literal.value, *intparse
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
    struct yfv_validator * validator,
    struct yfcs_binary * c, struct yfa_binary * a,
    struct yf_location * loc
) {
    /* First, if it's an assignment, the left side is a variable. */
    if (yfo_is_assign(c->op)) {
        if (c->left->type != YFCS_VALUE) {
            YF_PRINT_ERROR(
                "%s %d:%d: Left side of assignment must not be compound",
                loc->file, loc->line, loc->column
            );
            return 1;
        }
        if (c->left->expr.value.type != YFCS_IDENT) {
            YF_PRINT_ERROR(
                "%s %d:%d: Left side of assignment must be an identifier",
                loc->file, loc->line, loc->column
            );
            return 1;
        }
    }

    a->op = c->op;

    a->left = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->left)
        return 2;
    if (validate_expr_e(
        validator, &c->left->expr, a->left, loc
    )) {
        free(a->left);
        return 1;
    }

    a->right = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->right)
        return 2;
    if (validate_expr_e(
        validator, &c->right->expr, a->right, loc
    )) {
        free(a->right);
        a->right = NULL;
        return 1;
    }

    /* Check that the types are compatible. */
    if (yfs_output_diagnostics(
        yfse_get_expr_type(a->left, validator->fdata),
        yfse_get_expr_type(a->right, validator->fdata),
        validator->fdata,
        loc
    )) {
        return 1;
    }

    return 0;

}


static int validate_funccall(
    struct yfv_validator * validator,
    struct yfcs_funccall * c, struct yfa_funccall * a,
    struct yf_location * loc
) {


    struct yf_parse_node * carg;
    struct yf_ast_node   * aarg;
    struct yfsn_param    * param;
    struct yfs_type      * paramtype;

    int lgres;

    /* Make sure the function exists. */
    if (find_symbol(
        validator, &a->name, c->name.name
    ) == -1) {
        YF_PRINT_ERROR(
            "%s %d:%d: Unknown function '%s'",
            loc->file,
            loc->line,
            loc->column,
            c->name.name
        );
        return 1;
    }

    /* Make sure the function is actually a function. */
    if (a->name->type != YFS_FN) {
        YF_PRINT_ERROR(
            "%s %d:%d: Identifier '%s' is not a function",
            loc->file,
            loc->line,
            loc->column,
            c->name.name
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
                "%s %d:%d: too %s arguments in function call",
                loc->file,
                loc->line,
                loc->column,
                lgres ? "few" : "many"
            );
            yf_free(aarg);
            return 1;
        }
        if (lgres == -1) {
            yf_free(aarg);
            break;
        }

        if (validate_expr(
            validator, carg, aarg
        )) {
            yf_free(aarg);
            return 1;
        }

        if ( (paramtype =
            yfv_get_type_s(validator->fdata, param->type)
        ) == NULL) {
            YF_PRINT_ERROR(
                "%s %d:%d: Uncaught type error: unknown type '%s'",
                loc->file,
                loc->line,
                loc->column,
                param->type
            );
            return 1;
        }

        if (yfs_output_diagnostics(
            yfse_get_expr_type(&aarg->expr, validator->fdata),
            paramtype,
            validator->fdata,
            loc
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
static int validate_expr_e(
    struct yfv_validator * validator,
    struct yfcs_expr * c, struct yfa_expr * a,
    struct yf_location * loc
) {
    
    /* If this is unary - (just a value), ... */
    switch (c->type) {

    case YFCS_VALUE:
        a->type = YFA_VALUE;
        return validate_value(validator, &c->value, &a->as.value, loc);

    case YFCS_BINARY:
        a->type = YFA_BINARY;
        return validate_binary(validator, &c->binary, &a->as.binary, loc);

    case YFCS_FUNCCALL:
        a->type = YFA_FUNCCALL;
        return validate_funccall(validator, &c->call, &a->as.call, loc);
    }

    return 0;

}

int validate_expr(
    struct yfv_validator * validator,
    struct yf_parse_node * cin, struct yf_ast_node * ain
) {
    ain->type = YFA_EXPR;
    return validate_expr_e(validator, &cin->expr, &ain->expr, &cin->loc);
}
