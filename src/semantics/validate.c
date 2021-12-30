#include "validate.h"

#include <util/allocator.h>
#include <util/yfc-out.h>

/**
 * Forwards
 */

/* Any sort of "typical" transfer operation - takes a current file for local
 * decls, the project for all decls, and the two nodes to edit. */

/* I would use a typedef, but then the forwards would conflict. */
#define VDECL(name) static int name( \
    struct yf_parse_node *, \
    struct yf_ast_node *, \
    struct yf_project_compilation_data *, \
    struct yf_file_compilation_data *\
)

VDECL(validate_program);
VDECL(validate_funcdecl);
VDECL(validate_expr);
VDECL(validate_bstmt);

/* Takes an extra arg */
static int validate_vardecl(
    struct yf_parse_node * cin,
    struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata, bool global
);

int yfs_validate(
    struct yf_file_compilation_data * fdata,
    struct yf_project_compilation_data * pdata
) {
    return validate_program(
        &fdata->parse_tree, &fdata->ast_tree, pdata, fdata
    );
}

static int validate_program(
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {

    struct yf_parse_node * cnode;
    struct yf_ast_node * anode;
    struct yfcs_program * cprog;
    struct yfa_program * aprog;

    cprog = &cin->program;
    aprog = &ain->program;

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
                    cnode, anode, pdata, fdata, true
                )) {
                    free(anode);
                    return 1;
                }
                break;
            case YFCS_FUNCDECL:
                if (validate_funcdecl(
                    cnode, anode, pdata, fdata
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

/**
 * global - whether this is a global decl or not.
 */
static int validate_vardecl(
    struct yf_parse_node * cin,
    struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata, bool global
) {

    struct yf_sym * entry;
    struct yfcs_vardecl * c = &cin->vardecl;
    struct yfa_vardecl * a = &ain->vardecl;

    /* Make sure it isn't declared twice */
    if ( (
        entry = yfh_get(
            fdata->symtab.table, c->name.name
        )
    ) != NULL) {
        if (global) {
            /* The less-than is so that each double is only reported once. */
            /* This should never happen for global vars - these should have been
            caught in the symtab-building phase. */
            if (entry->line < cin->lineno) {
                YF_PRINT_ERROR(
                    "Uncaught duplicate declaration of symbol '%s'"
                    ", lines %d and %d",
                    c->name.name,
                    entry->line,
                    cin->lineno
                );
            }
        } else {
            /* Just a warning. */
            YF_PRINT_WARNING(
                "Global symbol '%s' (line %d)"
                "shadowed by local symbol (line %d)",
                c->name.name,
                entry->line,
                cin->lineno
            );
        }
        return 1;
    }

    /* Construct abstract instance */
    a->name = entry;
    a->type = entry->var.dtype;
    if (c->expr) {
        a->expr->type = YFA_EXPR;
        if (validate_expr(c->expr, a->expr, pdata, fdata))
            return 1;
    }
    
    return 0;

}

static int validate_expr(struct yf_parse_node * c, struct yf_ast_node * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata) {
    /* TODO */
    return 0;
}

static int validate_funcdecl(
    struct yf_parse_node * c, struct yf_ast_node * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {
    /* TODO */
    return 0;
}

static int validate_bstmt(struct yf_parse_node * c, struct yf_ast_node * a,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata) {
    /* TODO */
    return 0;
}
