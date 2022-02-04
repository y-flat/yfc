/**
 * All parse tree data structure definitions.
 * These are all context-free - this means no validation has been performed
 * whatsoever, and all we know is that these have been parsed correctly.
 * This closely mirrors the structure of the AST defined in api/abstract-tree.h
 * and some more informative comments are located there about what each node
 * does, but here are the basics:
 * - A node is one of several structures in a tagged union, representing every
 * possible lower-level structure of a program. Each node has a location for
 * error-reporting purposes, given to it by the lexer (whose tokens have
 * location information as well).
 * - There is no statement node, and the "parse_stmt" function really just looks
 * for whatever kind of node follows. As an example:
 *     x: int;
 *     x = 2;
 *     return x;
 * - Is just a list of nodes with types YFCS_VARDECL, YFCS_EXPR, and YFCS_RETURN
 * with no statement type needed.
 */

#ifndef API_CST_H
#define API_CST_H

#include <api/loc.h>
#include <api/operator.h>
#include <util/list.h>

struct yf_parse_node;

typedef char yfcs_databuf[256];

struct yfcs_identifier {
    struct yf_location loc;
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
    struct yf_location loc;
};

/**
 * Any call to a function. This is a type of expr!
 */
struct yfcs_funccall {
    struct yfcs_identifier name;
    struct yf_list args; /* A list of yf_parse_node */
};

struct yfcs_expr {

    union {
        struct yfcs_value value;
        struct yfcs_binary {
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
        YFCS_EMPTY,
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

    struct yf_location loc;

};

#endif /* API_CST_H */
