#include "validate.h"
#include "api/compilation-data.h"

#include <semantics/types.h>
#include <semantics/validate/validate-internal.h>

int yfs_validate(
    struct yf_compile_analyse_job * udata,
    struct yf_compilation_data * pdata
) {

    struct yfv_validator validator = {
        /* Root symbol table is the global scope of the program. */
        .current_scope = &udata->symtab,
        .udata         = udata,
        .pdata         = pdata
    };
    return validate_program(
        &validator, &udata->parse_tree, &udata->ast_tree
    );

}

int validate_node(
    struct yfv_validator * validator,
    struct yf_parse_node * csub, struct yf_ast_node * asub,
    struct yfs_type * for_bstmt1,
    int * for_bstmt2
) {

    switch (csub->type) {
    case YFCS_EXPR:
        return validate_expr(validator, csub, asub);
    case YFCS_VARDECL:
        return validate_vardecl(validator, csub, asub);
    case YFCS_FUNCDECL:
        return validate_funcdecl(validator, csub, asub);
    case YFCS_PROGRAM:
        return validate_program(validator, csub, asub);
    case YFCS_BSTMT:
        return validate_bstmt(validator, csub, asub, for_bstmt1, for_bstmt2);
    case YFCS_RET:
        return validate_return(validator, csub, asub, for_bstmt1, for_bstmt2);
    case YFCS_IF:
        return validate_if(validator, csub, asub, for_bstmt1, for_bstmt2);
    case YFCS_EMPTY:
        asub->type = YFA_EMPTY;
        return 0;
    default:
        YF_PRINT_ERROR("internal error: unknown CST node type");
        return 1;
    }

}

int validate_program(
    struct yfv_validator * validator,
    struct yf_parse_node * cin, struct yf_ast_node * ain
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
    
    /* Iterate through all decls, construct abstract instances of them, and move
    them into the abstract list. */
    YF_LIST_FOREACH(cprog->decls, cnode) {
        /* Construct abstract instance */
        anode = yf_malloc(sizeof (struct yf_ast_node));
        if (!anode)
            return 2;

        /* Validate */
        if (validate_node(validator, cnode, anode, NULL, NULL)) {
            yf_free(anode);
            anode = NULL;
            validator->error = 1;
            err = 1;
            /* No return, keep going to find more errors. */
        }

        /* Move to abstract list */
        if (anode)
            yf_list_add(&aprog->decls, anode);
    }

    return err;

}

int validate_return(
    struct yfv_validator * validator,
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yfs_type * type, int * returns
) {

    struct yfcs_return * c = &cin->ret;
    struct yfa_return  * a = &ain->ret;

    ain->type = YFA_RETURN;
    a->expr = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->expr)
        return 2;

    if (c->expr) {
        if (validate_expr(validator, c->expr, a->expr)) {
            validator->error = 1;
            return 1;
        }
    } else {
        a->expr = NULL;
    }

    if ( (type->primitive.size != 0) ?
            yfs_output_diagnostics(
                (a->expr != NULL)
                    ? yfse_get_expr_type(&a->expr->expr, validator->udata)
                    : yfv_get_type_s(validator->udata, "void"),
                type,
                validator->udata,
                &cin->loc
            )
        :   (a->expr != NULL)
    ) {
        validator->error = 1;
        return 1;
    }

    *returns = 1;

    return 0;

}
