/**
 * The tree structure that is created from a parse tree, with ALL identifiers
 * (except identifiers) being replaced with references. Any complete AST that
 * is in existence should have been fully validated.
 * 
 * This tree structure will look very similar to the CST ... because it is.
 */

#ifndef API_ABSTRACT_TREE_H
#define API_ABSTRACT_TREE_H

#include <api/operator.h>
#include <api/sym.h>
#include <util/list.h>

struct yf_ast_node;

/**
 * This represents a single, indivisible value.
 */
struct yfa_value {

    /**
     * Each single value is either an identifier (a, x.y.z, etc.) or a valuee
     * (like 2 or 57).
     */
    enum yfav_type {
        YFA_IDENT,
        YFA_LITERAL,
    } type;

    /**
     * The core is either a reference to an identifier, or one of the two types
     * of literal - a number or a boolean. In the future, there will be more
     * types, like strings and floating-point numbers. The future representation
     * of arrays is still an open question.
     */
    union {
        struct yf_sym * identifier;
        struct {
            enum yfa_lit_type {
                YFAL_NUM,
                YFAL_BOOL,
            } type;
            union {
                int val;
            };
        } literal;
    } as;

};

/**
 * A call to a function, made of a reference to the function along with a list
 * of its arguments.
 */
struct yfa_funccall {
    struct yf_sym * name;
    struct yf_list args; /* A list of yf_ast_node */
};

/**
 * An expression is one of three types: a function call, a binary expression
 * with an operator and two operands, or a value.
 */
struct yfa_expr {
    union {
        struct yfa_value value;
        struct yfa_binary {
            struct yfa_expr *left;
            struct yfa_expr *right;
            enum yf_operator op;
        } binary;
        struct yfa_funccall call;
    } as;

    enum {
        YFA_VALUE,
        YFA_BINARY,
        YFA_FUNCCALL,
    } type;
    
};

/**
 * A return statement only needs the expression it returns.
 */
struct yfa_return {
    struct yf_ast_node * expr;
};

/**
 * A node representing a variable decl IN THE CODE, NOT in any abstract
 * symbol table. This node exists only to generate the appropriate code and is
 * in no way a dependence of any other validation process.
 * Made of a variable symbol which is referenced, and the expression being
 * assigned to it, if any.
 */
struct yfa_vardecl {
    struct yf_sym * name;
    /* Can be NULL, and is always of type expr, is the right-hand side */
    struct yf_ast_node * expr;
};

/**
 * Same deal as vardecl - this only exists to output the appropriate code.
 */
struct yfa_funcdecl {
    struct yf_sym * name;
    struct yfs_type * ret; /* The return type */
    /* All parameters are stored as vardecls. expr WILL be null for these. */
    struct yf_list params;
    struct yf_ast_node * body; /* The function body */
    /* The parameter scope */
    struct yfs_symtab * param_scope;
};

/**
 * A program is just a list of top-level declarations.
 */
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

/**
 * An if statement is composed of a condition, the code which runs if the
 * condition is satisfied, and an optional else clause (NULL if no else).
 */
struct yfa_if {
    struct yf_ast_node * cond;
    struct yf_ast_node * code;
    struct yf_ast_node * elsebranch;
};

/**
 * Just a tagged union with all the possible types of nodes.
 */
struct yf_ast_node {
    
    enum {
        YFA_EMPTY,
        YFA_EXPR,
        YFA_VARDECL,
        YFA_FUNCDECL,
        YFA_PROGRAM,
        YFA_BSTMT,
        YFA_RETURN,
        YFA_IF,
    } type;

    union {
        struct yfa_expr expr;
        struct yfa_vardecl vardecl;
        struct yfa_funcdecl funcdecl;
        struct yfa_program program;
        struct yfa_bstmt bstmt;
        struct yfa_return ret;
        struct yfa_if ifstmt;
    };

};

void yf_cleanup_ast(struct yf_ast_node * node);

#endif /* API_ABSTRACT_TREE_H */
