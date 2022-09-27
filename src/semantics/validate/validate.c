#include "validate.h"
#include "api/compilation-data.h"

#include <semantics/types.h>
#include <semantics/validate/validate-internal.h>

static void add_type(
    struct yf_compile_analyse_job * udata,
    char * name, int size, enum yfpt_format fmt
) {

    struct yfs_type * type = yf_malloc(sizeof (struct yfs_type));
    type->primitive.size = size;
    type->kind = YFST_PRIMITIVE;
    type->primitive.type = fmt;
    type->name = name;
    yfv_add_type(udata, type);

}

static void yfv_add_builtin_types(struct yf_compile_analyse_job * udata) {

    /* All types are signed for now - unsigned types are not yet supported. */

    /* "standard" types. */
    add_type(udata, "char",        8, YFS_INT  );
    add_type(udata, "short",      16, YFS_INT  );
    add_type(udata, "int",        32, YFS_INT  );
    add_type(udata, "long",       64, YFS_INT  );
    add_type(udata, "void",        0, YFS_NONE );
    add_type(udata, "float",      32, YFS_FLOAT);
    add_type(udata, "double",     64, YFS_FLOAT);

    /* Convenience types. */
    add_type(udata, "i16",        16, YFS_INT  );
    add_type(udata, "i32",        32, YFS_INT  );
    add_type(udata, "i64",        64, YFS_INT  );
    add_type(udata, "f16",        16, YFS_FLOAT);
    add_type(udata, "f32",        32, YFS_FLOAT);
    add_type(udata, "f64",        64, YFS_FLOAT);

    /* We're considering bool to be one bit for conversion purposes. */
    add_type(udata, "bool",       1,  YFS_INT  );

}

int yfs_validate(
    struct yf_compile_analyse_job * udata,
    struct yf_compilation_data * pdata
) {

    yfh_init(&udata->types.table);
    yfv_add_builtin_types(udata);
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
