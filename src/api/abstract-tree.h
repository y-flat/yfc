/**
 * The tree structure that is created from a parse tree, with ALL identifiers
 * (except identifiers) being replaced with references. Any complete AST that
 * is in existence should have been fully validated.
 * 
 * As C is a fully explicit language, there will be nothing explicit about this
 * tree - all default parameters are filled in, all constructors are called
 * explicitly, and so forth.
 * 
 * This tree structure will look very similar to the CST ... because it is.
 */

#ifndef API_ABSTRACT_TREE_H
#define API_ABSTRACT_TREE_H

#include <api/operator.h>
#include <api/sym.h>
#include <util/list.h>

struct yf_ast_node;

struct yfa_value {

    enum {
        YFA_IDENT,
        YFA_LITERAL,
    } type;

    union {
        struct yf_sym * identifier;
        struct {
            enum {
                YFCS_NUM, /* TODO */
            } type;
            union {
                int val;
            };
        } literal;
    } as;

};

struct yfa_expr {
    union {
        struct yfa_value value;
        struct {
            struct yfa_expr *left;
            struct yfa_expr *right;
            enum yf_operator op;
        } binary;
    } as;

    enum {
        YFA_VALUE,
        YFA_BINARY,
    } type;
    
};

struct yfa_vardecl {
    struct yf_sym * name;
    /* Can be NULL, and is always of type expr, is the right-hand side */
    struct yf_ast_node * expr;
};

struct yfa_funcdecl {
    struct yf_sym * name;
    struct yfs_type ret; /* The return type */
    /* All parameters are stored as vardecls. expr WILL be null for these. */
    struct yf_list params;
    struct yf_ast_node * body; /* The function body */
};

struct yfa_program {
    struct yf_list decls;
};

struct yfa_bstmt {

    struct yf_list stmts;

    /* Each block statement has a scope associated with it - or, its own symbol
     * table. Searches for references start here.
     */
    struct yfs_symtab * symtab;

};

struct yf_ast_node {
    
    enum {
        YFA_EXPR,
        YFA_VARDECL,
        YFA_FUNCDECL,
        YFA_PROGRAM,
        YFA_BSTMT,
    } type;

    union {
        struct yfa_expr expr;
        struct yfa_vardecl vardecl;
        struct yfa_funcdecl funcdecl;
        struct yfa_program program;
        struct yfa_bstmt bstmt;
    };

};

#endif /* API_ABSTRACT_TREE_H */
