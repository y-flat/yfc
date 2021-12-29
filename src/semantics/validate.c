#include "validate.h"

#include <util/allocator.h>
#include <util/yfc-out.h>

/**
 * Forwards
 */
static int validate_program(struct yfcs_program * c, struct yfa_program * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata);
static int validate_vardecl(struct yf_parse_node * c, struct yfa_vardecl * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata);
static int validate_expr(struct yf_parse_node * c, struct yfa_expr * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata);
static int validate_funcdecl(struct yf_parse_node * c, struct yfa_funcdecl * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata);
static int validate_bstmt(struct yf_parse_node * c, struct yfa_bstmt * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata);

int yfs_validate(
    struct yf_file_compilation_data * fdata,
    struct yf_project_compilation_data * pdata
) {
    return validate_program(
        &fdata->parse_tree.as.program, &fdata->ast_tree.program, pdata, fdata
    );
}

static int validate_program(
    struct yfcs_program * cprog, struct yfa_program * aprog,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {

    struct yf_parse_node * cnode;
    struct yf_ast_node * anode;

    yf_list_init(&aprog->decls);
    yf_list_reset(&cprog->decls);
    
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
        switch (cnode->type) {
            case YFCS_VARDECL:
                if (validate_vardecl(
                    cnode, &anode->vardecl, pdata, fdata
                )) {
                    free(anode);
                    return 1;
                }
                break;
            case YFCS_FUNCDECL:
                if (validate_funcdecl(
                    cnode, &anode->funcdecl, pdata, fdata
                )) {
                    free(anode);
                    return 1;
                }
                break;
            default:
                YF_PRINT_ERROR("Unknown node type in internal CST tree");
                return 2;
        }

        /* Move to abstract list */
        yf_list_add(&aprog->decls, anode);

        /* Keep going */
        yf_list_next(&cprog->decls);

    }

    return 0;

}

static int validate_vardecl(struct yf_parse_node * c, struct yfa_vardecl * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata) {

    struct yf_sym * entry;
    struct yfcs_vardecl * cdecl = &c->as.vardecl;

    /* Make sure it isn't declared twice */
    if ( (
        entry = yfh_get(
            fdata->symtab.table, cdecl->name.name.databuf
        )
    ) != NULL) {
        YF_PRINT_ERROR(
            "Duplicate declaration of symbol '%s', lines %d and %d",
            cdecl->name.name.databuf,
            entry->line,
            c->lineno
        );
        return 1;
    }

    /* Construct abstract instance */
    a->name = entry;
    a->type = entry->var.dtype;
    if (validate_expr(cdecl->expr, &a->expr->expr, pdata, fdata))
        return 1;
    
    return 0;

}

static int validate_expr(struct yf_parse_node * c, struct yfa_expr * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata) {
    /* TODO */
    return 0;
}

static int validate_funcdecl(
    struct yf_parse_node * c, struct yfa_funcdecl * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {
    /* TODO */
    return 0;
}

static int validate_bstmt(struct yf_parse_node * c, struct yfa_bstmt * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata) {
    /* TODO */
    return 0;
}
