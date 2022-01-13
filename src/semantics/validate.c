#include "validate.h"

#include <api/operator.h>
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
VDECL(validate_node);

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

/**
 * Create a new scope - return 0 on success, 1 on failure (memory error).
 * The root of the created symtab is set to the current scope, and the current
 * scope is also set to the new scope.
 */
static int enter_scope(struct yfs_symtab ** stuff) {

    struct yfs_symtab * old_symtab, * new_symtab;
    old_symtab = current_scope;

    new_symtab = yf_malloc(sizeof (struct yfs_symtab));
    if (!new_symtab) {
        return 1;
    }
    new_symtab->table = yfh_new();
    if (!new_symtab->table) {
        free(new_symtab);
        return 1;
    }

    new_symtab->parent = old_symtab;
    current_scope = new_symtab;

    if (stuff)
        *stuff = new_symtab;
    
    return 0;

}

void add_type(struct yf_file_compilation_data * fdata, char * name, int size) {
    struct yfs_type * type = yf_malloc(sizeof (struct yfs_type));
    type->primitive.size = size;
    type->kind = YFST_PRIMITIVE;
    yfh_set(fdata->types.table, name, type);
}

void yfv_add_builtin_types(struct yf_file_compilation_data * fdata) {
    add_type(fdata, "char",        8);
    add_type(fdata, "short",      16);
    add_type(fdata, "int",        32);
    add_type(fdata, "long",       32);
    add_type(fdata, "void",        0);
}

int yfs_validate(
    struct yf_file_compilation_data * fdata,
    struct yf_project_compilation_data * pdata
) {
    fdata->types.table = yfh_new();
    yfv_add_builtin_types(fdata);
    return validate_program(
        &fdata->parse_tree, &fdata->ast_tree, pdata, fdata
    );
}

static int validate_node(
    struct yf_parse_node * csub, struct yf_ast_node * asub,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {
    switch (csub->type) {
    case YFCS_EXPR:
        return validate_expr(csub, asub, pdata, fdata);
    case YFCS_VARDECL:
        return validate_vardecl(csub, asub, pdata, fdata);
    case YFCS_FUNCDECL:
        return validate_funcdecl(csub, asub, pdata, fdata);
    case YFCS_PROGRAM:
        return validate_program(csub, asub, pdata, fdata);
    case YFCS_BSTMT:
        return validate_bstmt(csub, asub, pdata, fdata);
    default:
        YF_PRINT_ERROR("internal error: unknown CST node type");
        return 1;
    } 
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
        if (validate_node(cnode, anode, pdata, fdata)) {
            yf_free(anode);
            /* return 1; */
            /* No return, keep going to find more errors. */
        }

        /* Move to abstract list */
        yf_list_add(&aprog->decls, anode);

        /* Keep going */
        yf_list_next(&cprog->decls);

    }

    return 0;

}

/**
 * Takes an input of a vardecl node.
 */
static int validate_vardecl(
    struct yf_parse_node * cin,
    struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {

    struct yf_sym * entry;

    struct yfcs_vardecl * c = &cin->vardecl;
    struct yfa_vardecl  * a = &ain->vardecl;

    int ssym;
    bool global = (current_scope == &fdata->symtab);

    /* Make sure it isn't declared twice */
    /* A variable is redeclared in the current scope. Whoops! */
    /* Only report this for non-global decls, those have been caught already
    in symtab building. */
    if ( (
        ssym = find_symbol(&entry, current_scope, c->name.name)
    ) != -1 && !global) {
        if (ssym == 0) {
            YF_PRINT_ERROR(
                "Duplicate declaration of symbol '%s'"
                ", lines %d and %d",
                c->name.name,
                entry->line,
                cin->lineno
            );
        } else {
            /* Just a warning about shadowing. */
            YF_PRINT_WARNING(
                "Global symbol '%s' (line %d) "
                "shadowed by local symbol (line %d)",
                c->name.name,
                entry->line,
                cin->lineno
            );
        }
        return 1;
    }

    /* Construct abstract instance, ONLY if non-global */

    a->name = yf_malloc(sizeof (struct yf_sym));
    if (!a->name)
        return 2;

    a->name->type = YFS_VAR;

    /* Verify type */
    /* We have to check that the type is valid here, because the type table
    doesn't exist during the symtab-building phase. */
    if ( (a->name->var.dtype = yfh_get(fdata->types.table, c->type.databuf)) == NULL) {
        YF_PRINT_ERROR(
            "Unknown type '%s' in declaration of '%s' (line %d)",
            c->type.databuf,
            c->name.name,
            cin->lineno
        );
        free(a->name);
        return 1;
    }

    /* Variables can't have type "void" */
    if (
        a->name->var.dtype->kind == YFST_PRIMITIVE
        && a->name->var.dtype->primitive.size == 0
    ) {
        YF_PRINT_ERROR(
            "Variable '%s' has type 'void' (line %d)",
            c->name.name,
            cin->lineno
        );
        free(a->name);
        return 1;
    }
    
    a->name->line = cin->lineno;

    if (c->expr) {
        a->expr = yf_malloc(sizeof (struct yf_ast_node));
        if (!a->expr)
            return 2;
        a->expr->type = YFA_EXPR;
        if (validate_expr(c->expr, a->expr, pdata, fdata))
            return 1;
    } else {
        a->expr = NULL;
    }

    /* Add to symbol table UNLESS it is global scope. */
    /* The global scope symtab is already set up. */
    if (!global) {
        yfh_set(current_scope->table, c->name.name, a->name);
    } else {
        /* Free the name, since it was only needed for type checking. */
        free(a->name);
        /* If it's global, set "name" to point to the global symbol. */
        a->name = entry;
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
    if (c->type == YFCS_VALUE) {

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
        } else {
            /* TODO - parse literal */
        }

    } else {
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

    }

    return 0;

}

static int validate_expr(struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {
    return validate_expr_e(&cin->expr, &ain->expr, pdata, fdata, cin->lineno);
}

static int validate_funcdecl(
    struct yf_parse_node * cin, struct yf_ast_node * ain,
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata
) {
    
    struct yfcs_funcdecl  * c = &cin->funcdecl;
    struct yfa_funcdecl   * a = &ain->funcdecl;
    struct yf_parse_node  * cv;
    struct yf_ast_node    * av;

    int ssym;

    /* Functions are only global. */
    if ( (
        ssym = find_symbol(&a->name, current_scope, c->name.name)
    ) == -1) {
        /* Uh oh ... */
        YF_PRINT_ERROR("internal error: symbol not found");
        return 2;
    }

    /* Now, validate the argument list. */
    /* Also, open a new scope for arguments. */
    enter_scope(NULL);

    /* Add the arguments to the scope. */
    yf_list_reset(&c->params);
    yf_list_init(&a->params);
    for (;;) {
        if (yf_list_get(&c->params, (void**) &cv) == -1)
            break;
        av = yf_malloc(sizeof (struct yfa_vardecl));
        if (!av)
            return 2;
        if (validate_vardecl(cv, av, pdata, fdata))
            return 1;
        yf_list_add(&a->params, av);
        yf_list_next(&c->params);
    }

    /* Validate the return type. */
    if ((a->ret = yfh_get(fdata->types.table, c->ret.databuf)) == NULL) {
        YF_PRINT_ERROR(
            "Unknown return type '%s' of function '%s' (line %d)",
            c->ret.databuf,
            c->name.name,
            cin->lineno
        );
        return 1;
    }

    a->body = yf_malloc(sizeof (struct yf_ast_node));
    if (!a->body)
        return 2;

    /* Now, validate the body. */
    if (validate_bstmt(c->body, a->body, pdata, fdata))
        return 1;

    /* Close the scope. */
    current_scope = current_scope->parent;

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
    enter_scope(&a->symtab);

    /* Validate each statement */
    yf_list_reset(&c->stmts);

    for (;;) {

        /* Get element */
        if (yf_list_get(&c->stmts, (void **) &csub) == -1) break;
        if (!csub) break;
        
        /* Construct abstract instance */
        asub = yf_malloc(sizeof (struct yf_ast_node));
        if (!asub)
            return 2;

        /* Validate */
        if (validate_node(csub, asub, pdata, fdata)) {
            yf_free(asub);
            /* return 1; */ /* Keep going */
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
