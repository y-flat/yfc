#include <semantics/validate/validate-internal.h>

#include <ctype.h>
#include <string.h>

#include <semantics/types.h>

/**
 * Takes in parameters of expr, rather than node, types.
 */
static int validate_expr_e(struct yfcs_expr * c, struct yfa_expr * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata, int lineno
) {

    char * intparse;
    int dig;

    struct yf_parse_node * carg;
    struct yf_ast_node   * aarg;
    struct yfsn_param    * param;
    struct yfs_type      * paramtype;
    int lgres;
    
    /* If this is unary - (just a value), ... */
    switch (c->type) {
    case YFCS_VALUE:

        a->type = YFA_VALUE;
        
        /* If an identifier, make sure it actually exists. */
        if (c->value.type == YFCS_IDENT) {
            if (find_symbol(&a->as.value.as.identifier, current_scope, c->value.identifier.name) == -1) {
                YF_PRINT_ERROR(
                    "Unknown identifier '%s' (line %d)",
                    c->value.identifier.name,
                    lineno
                );
                return 1;
            }
            a->as.value.type = YFA_IDENT;
        } else {

            a->as.value.type = YFA_LITERAL;

            if (isdigit(c->value.literal.value[0])) {

                /* Parse integer literal */
                a->as.value.as.literal.type = YFAL_NUM;
                a->as.value.as.literal.val = 0;

                for (intparse = c->value.literal.value; *intparse; ++intparse) {
                    dig = *intparse - '0';
                    if (dig < 0 || dig > 9) {
                        YF_PRINT_ERROR(
                            "Invalid literal '%s' (line %d), "
                            "found invalid character '%c' in int literal",
                            c->value.literal.value, lineno, *intparse
                        );
                        return 1;
                    }
                    a->as.value.as.literal.val *= 10;
                    a->as.value.as.literal.val += dig;
                }

                return 0;

            }

            /* Check for 'true' or 'false' */
            if (strcmp(c->value.literal.value, "true") == 0) {
                a->as.value.as.literal.type = YFAL_BOOL;
                a->as.value.as.literal.val = 1;
                return 0;
            }
            if (strcmp(c->value.literal.value, "false") == 0) {
                a->as.value.as.literal.type = YFAL_BOOL;
                a->as.value.as.literal.val = 0;
                return 0;
            }

        }

        break;

    case YFCS_BINARY:
        /* It's a binary expression. */
        a->type = YFA_BINARY;

        /* First, if it's an assignment, the left side is a variable. */
        if (yfo_is_assign(c->binary.op)) {
            if (c->binary.left->type != YFCS_VALUE) {
                YF_PRINT_ERROR(
                    "Left side of assignment must not be compound (line %d)",
                    lineno
                );
                return 1;
            }
            if (c->binary.left->expr.value.type != YFCS_IDENT) {
                YF_PRINT_ERROR(
                    "Left side of assignment must be an identifier (line %d)",
                    lineno
                );
                return 1;
            }
        }

        a->as.binary.op = c->binary.op;

        a->as.binary.left = yf_malloc(sizeof (struct yf_ast_node));
        if (!a->as.binary.left)
            return 2;
        if (validate_expr_e(
            &c->binary.left->expr, a->as.binary.left, pdata, fdata, lineno
        ))
            return 1;

        a->as.binary.right = yf_malloc(sizeof (struct yf_ast_node));
        if (!a->as.binary.right)
            return 2;
        if (validate_expr_e(
            &c->binary.right->expr, a->as.binary.right, pdata, fdata, lineno
        ))
            return 1;

        /* Check that the types are compatible. */
        if (yfs_output_diagnostics(
            yfse_get_expr_type(a->as.binary.left, fdata),
            yfse_get_expr_type(a->as.binary.right, fdata),
            fdata,
            lineno
        )) {
            return 1;
        }

        break;

    case YFCS_FUNCCALL:
        /* Make sure the function exists. */
        if (find_symbol(
            &a->as.call.name, current_scope, c->call.name.name
        ) == -1) {
            YF_PRINT_ERROR(
                "Unknown function '%s' (line %d)",
                c->call.name.name,
                c->call.name.lineno
            );
            return 1;
        }

        a->type = YFA_FUNCCALL;

        /* Make sure the function is actually a function. */
        if (a->as.call.name->type != YFS_FN) {
            YF_PRINT_ERROR(
                "Identifier '%s' is not a function (line %d)",
                c->call.name.name,
                c->call.name.lineno
            );
            return 1;
        }

        /* Go through the arguments and add them to the list, while making sure
         * the types are compatible for each one and the number of arguments
         * matches.
         */
        yf_list_init(&a->as.call.args);
        yf_list_reset(&a->as.call.name->fn.params);
        yf_list_reset(&c->call.args);
        for (;;) {

            aarg = yf_malloc(sizeof (struct yf_ast_node));
            if (!aarg)
                return 2;

            if (
                yf_list_get(&a->as.call.name->fn.params, (void **) &param) !=
                (lgres = yf_list_get(&c->call.args, (void **) &carg))
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

            yf_list_add(&a->as.call.args, aarg);
            yf_list_next(&c->call.args);
            yf_list_next(&a->as.call.name->fn.params);

        }

        break;

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
