/**
 * The tree structure that is created from a parse tree, with ALL identifiers
 * (except identifiers) being replaced with references. Any complete AST that
 * is in existence should have been fully validated.
 * 
 * As C is a fully explicit language, there will be nothing explicit about this
 * tree - all default parameters are filled in, all constructors are called
 * explicitly, and so forth.
 */

#ifndef API_ABSTRACT_TREE_H
#define API_ABSTRACT_TREE_H

struct yfa_expr {
    /* TODO */
};

struct yfa_vardecl {
    /* TODO */
};

struct yfa_funcdecl {
    /* TODO */
};

struct yfa_program {
    /* TODO */
};

struct yfa_bstmt {
    /* TODO */
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
