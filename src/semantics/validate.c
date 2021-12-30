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
VDECL(validate_vardecl);

/**
 * Internal - the innermost scope we have open. TODO - un-static this.
 */
static struct yfs_symtab * current_scope;

/**
 * Search for a symbol with the given name. Return "depth" - innermost scope is
 * 0, the next-enclosing is 1, etc. If not found, -1.
 */
static int find_symbol(
    struct yf_sym ** sym, struct yfs_symtab * symtab,
    char * name
) {
    int depth = 0;
    while (symtab != NULL) {
        if ( (*sym = yfh_get(symtab->table, name)) != NULL) {
            return depth;
        }
        depth++;
        symtab = symtab->parent;
    }
    return -1;
}

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

    /* Initialize the scope */
    current_scope = &fdata->symtab;
    
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
                    cnode, anode, pdata, fdata
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
    struct yf_file_compilation_data * fdata
) {

    struct yf_sym * entry;
    struct yfcs_vardecl * c = &cin->vardecl;
    struct yfa_vardecl * a = &ain->vardecl;

    int ssym;

    /* Make sure it isn't declared twice */
    if ( (
        ssym = find_symbol(&entry, current_scope, c->name.name)
    ) != -1) {
        /* A variable is redeclared in the current scope. Whoops! */
        if (ssym == 0) {
            /* The less-than is so that each double is only reported once. */
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
            /* Just a warning about shadowing. */
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

    /* If lookup succeeded, point this to said symbol. */
    /* Otherwise, create a new symbol! */
    if (entry) {
        a->name = entry;
    } else {
        a->name = yf_malloc(sizeof (struct yf_sym));
        if (!a->name)
            return 2;
        a->name->var.dtype =  /* c->type */
            (struct yfs_type) { NULL }; /* TODO - get type lookup */
        a->name->line = cin->lineno;
    }
    if (c->expr) {
        a->expr->type = YFA_EXPR;
        if (validate_expr(c->expr, a->expr, pdata, fdata))
            return 1;
    }

    /* Add to symbol table UNLESS it is global scope. */
    /* The global scope symtab is already sete up. */
    if (current_scope != &fdata->symtab) {
        yfh_set(current_scope->table, c->name.name, a->name);
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

static int validate_bstmt(struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {

    struct yfcs_bstmt * c = &cin->bstmt;
    struct yfa_bstmt * a = &ain->bstmt;

    struct yf_parse_node * csub;
    struct yf_ast_node * asub;
    
    /* Create a symbol table for this scope */
    a->symtab = yf_malloc(sizeof (struct yfs_symtab));
    if (!a->symtab)
        return 2;
    a->symtab->table = yfh_new();
    a->symtab->parent = current_scope;
    current_scope = a->symtab;

    /* Validate each statement */
    yf_list_reset(&c->stmts);

    for(;;) {

        /* Get element */
        if (yf_list_get(&c->stmts, (void **) &csub) == -1) break;
        if (!csub) break;
        
        /* Construct abstract instance */
        asub = yf_malloc(sizeof (struct yf_ast_node));
        if (!asub)
            return 2;

        /* Validate */
        /* TODO - this switch statement is a mess. Maybe factor it out. */
        switch (csub->type) {
        case YFCS_EXPR:
            if (validate_expr(csub, asub, pdata, fdata))
                return 1;
        case YFCS_VARDECL:
            if (validate_vardecl(csub, asub, pdata, fdata))
                return 1;
        case YFCS_FUNCDECL:
            if (validate_funcdecl(csub, asub, pdata, fdata))
                return 1;
        case YFCS_PROGRAM:
            if (validate_program(csub, asub, pdata, fdata))
                return 1;
        case YFCS_BSTMT:
            if (validate_bstmt(csub, asub, pdata, fdata))
                return 1;
        }

        /* Move to abstract list */
        yf_list_add(&a->stmts, asub);

        /* Keep going */
        yf_list_next(&c->stmts);
        
    }

    /* Now, pop this scope off the stack. */
    current_scope = current_scope->parent;

    return 0;

}
