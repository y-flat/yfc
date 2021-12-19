/**
 * All parse tree data structure definitions.
 */

#ifndef API_CST_H
#define API_CST_H

#include <util/list.h>

struct yf_parse_node;

struct yfcs_databuf {
    char databuf[256];
    int datalen;
};

struct yfcs_identifier {
    struct yfcs_databuf name;
};

struct yfcs_literal {
    struct yfcs_databuf value;
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
    char databuf[256];
};

/**
 * TODO - future binary expr parsing
 */
struct yfcs_expr {
    struct yfcs_value value;
};

struct yfcs_vardecl {
    struct yfcs_identifier name;
    struct yfcs_type type;
    /* Can be NULL, and is always of type expr, is the right-hand side */
    struct yfcs_expr expr;
};

struct yfcs_funcdecl {
    struct yfcs_identifier name;
    struct yfcs_type ret; /* The return type */
    /* All parameters aree stored as vardecls. expr WILL be null for these. */
    struct yf_list params;
};

struct yf_parse_node {
    
    enum {
        YFCS_EXPR,
        YFCS_VARDECL,
        YFCS_FUNCDECL,
    } as;

    union {
        struct yfcs_expr expr;
        struct yfcs_vardecl vardecl;
        struct yfcs_funcdecl funcdecl;
    };

};

#endif /* API_CST_H */
