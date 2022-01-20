/**
 * All parse tree data structure definitions.
 */

#ifndef API_CST_H
#define API_CST_H

#include <api/operator.h>
#include <util/list.h>

struct yf_parse_node;

typedef char yfcs_databuf[256];

struct yfcs_identifier {
    int lineno, colno;
    yfcs_databuf name;
};

struct yfcs_literal {
    yfcs_databuf value;
};

/* Any single value, whether an identifier like "a.b" or a literal like 2. */
struct yfcs_value {

    enum {
        YFCS_IDENT,
        YFCS_LITERAL,
    } type;

    union {
        struct yfcs_identifier identifier;
        struct yfcs_literal literal;
    };

};

/* Types are stored as strings in the concrete syntax tree, even future complex
 * types like "class<type> follows constraint". */
struct yfcs_type {
    yfcs_databuf databuf;
    int lineno, colno;
};

/**
 * Any call to a function. This is a type of expr!
 */
struct yfcs_funccall {
    struct yfcs_identifier name;
    struct yf_list args; /* A list of yf_parse_node */
};

/**
 * TODO - funccalls, etc.
 */
struct yfcs_expr {

    union {
        struct yfcs_value value;
        struct {
            struct yf_parse_node *left;
            struct yf_parse_node *right;
            enum yf_operator op;
        } binary;
        struct yfcs_funccall call;
    };

    enum {
        YFCS_VALUE,
        YFCS_BINARY,
        YFCS_FUNCCALL,
    } type;
    
};

/**
 * A return statement. Expr may be null if the statement is "return;"
 */
struct yfcs_return {
    struct yf_parse_node * expr;
};

struct yfcs_vardecl {
    struct yfcs_identifier name;
    struct yfcs_type type;
    /* Can be NULL, and is always of type expr, is the right-hand side */
    struct yf_parse_node * expr;
};

struct yfcs_funcdecl {
    struct yfcs_identifier name;
    struct yfcs_type ret; /* The return type */
    /* All parameters are stored as parse_node. expr WILL be null for these. */
    struct yf_list params;
    struct yf_parse_node * body; /* The function body */
};

struct yfcs_if {
    struct yf_parse_node * cond;
    struct yf_parse_node * code;
    struct yf_parse_node * elsebranch;
};

struct yfcs_program {
    struct yf_list decls;
};

/**
 * A block statement.
 */
struct yfcs_bstmt {
    struct yf_list stmts;
};

struct yf_parse_node {
    
    enum yfcs_node_type {
        YFCS_EXPR,
        YFCS_VARDECL,
        YFCS_FUNCDECL,
        YFCS_PROGRAM,
        YFCS_BSTMT,
        YFCS_RET,
        YFCS_IF,
    } type;

    union {
        struct yfcs_expr expr;
        struct yfcs_vardecl vardecl;
        struct yfcs_funcdecl funcdecl;
        struct yfcs_program program;
        struct yfcs_bstmt bstmt;
        struct yfcs_return ret;
        struct yfcs_if ifstmt;
    };

    int lineno, colno;

};

#endif /* API_CST_H */
