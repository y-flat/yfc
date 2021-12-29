#include "validate.h"

#include <util/allocator.h>
#include <util/yfc-out.h>

/**
 * Forwards
 */
static int validate_program(struct yfcs_program * c, struct yfa_program * a,
    struct yf_project_compilation_data * pdata);
static int validate_vardecl(struct yfcs_vardecl * c, struct yfa_vardecl * a,
    struct yf_project_compilation_data * pdata);
static int validate_expr(struct yfcs_expr * c, struct yfa_expr * a,
    struct yf_project_compilation_data * pdata);
static int validate_funcdecl(struct yfcs_funcdecl * c, struct yfa_funcdecl * a,
    struct yf_project_compilation_data * pdata);
static int validate_bstmt(struct yfcs_bstmt * c, struct yfa_bstmt * a,
    struct yf_project_compilation_data * pdata);

int yfs_validate(
    struct yf_file_compilation_data * fdata,
    struct yf_project_compilation_data * pdata
) {
    return validate_program(
        &fdata->parse_tree.as.program, &fdata->ast_tree.program, pdata
    );
}

static int validate_program(
    struct yfcs_program * cprog, struct yfa_program * aprog,
    struct yf_project_compilation_data * pdata
) {

    struct yf_parse_node * cnode;
    struct yf_ast_node * anode;

    yf_list_init(&aprog->decls);
    
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
                    &cnode->as.vardecl, &anode->vardecl, pdata
                )) {
                    free(anode);
                    return 1;
                }
                break;
            case YFCS_FUNCDECL:
                if (validate_funcdecl(
                    &cnode->as.funcdecl, &anode->funcdecl, pdata
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

static int validate_vardecl(struct yfcs_vardecl * c, struct yfa_vardecl * a,
    struct yf_project_compilation_data * pdata) {
    /* TODO */
    return 0;
}

static int validate_expr(struct yfcs_expr * c, struct yfa_expr * a,
    struct yf_project_compilation_data * pdata) {
    /* TODO */
    return 0;
}

static int validate_funcdecl(
    struct yfcs_funcdecl * c, struct yfa_funcdecl * a,
    struct yf_project_compilation_data * pdata
) {
    /* TODO */
    return 0;
}

static int validate_bstmt(struct yfcs_bstmt * c, struct yfa_bstmt * a,
    struct yf_project_compilation_data * pdata) {
    /* TODO */
    return 0;
}
