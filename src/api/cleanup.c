/**
 * All "cleanup" functions - for freeing up unused API structures.
 */

#include <api/abstract-tree.h>
#include <api/concrete-tree.h>
#include <util/allocator.h>

void yf_cleanup_aexpr(struct yfa_expr * node);
void yf_cleanup_avardecl(struct yfa_vardecl * node);
void yf_cleanup_afuncdecl(struct yfa_funcdecl * node);
void yf_cleanup_aprogram(struct yfa_program * node);
void yf_cleanup_abstmt(struct yfa_bstmt * node);
void yf_cleanup_areturn(struct yfa_return * node);
void yf_cleanup_aif(struct yfa_if * node);

void yf_cleanup_anode(struct yf_ast_node * node, int free_node) {
    switch (node->type) {
    case YFA_EMPTY:
        break;
    case YFA_EXPR:
        yf_cleanup_aexpr(&node->expr);
        break;
    case YFA_VARDECL:
        yf_cleanup_avardecl(&node->vardecl);
        break;
    case YFA_FUNCDECL:
        yf_cleanup_afuncdecl(&node->funcdecl);
        break;
    case YFA_PROGRAM:
        yf_cleanup_aprogram(&node->program);
        break;
    case YFA_BSTMT:
        yf_cleanup_abstmt(&node->bstmt);
        break;
    case YFA_RETURN:
        yf_cleanup_areturn(&node->ret);
        break;
    case YFA_IF:
        yf_cleanup_aif(&node->ifstmt);
        break;
    }
    if (free_node)
        yf_free(node);
}

void yf_cleanup_aexpr(struct yfa_expr * node) {
    struct yf_ast_node * anode;
    switch (node->type) {
    case YFA_VALUE:
        break;
    case YFA_BINARY:
        yf_cleanup_aexpr(node->as.binary.left);
        yf_free(node->as.binary.left);
        yf_cleanup_aexpr(node->as.binary.right);
        yf_free(node->as.binary.right);
        break;
    case YFA_FUNCCALL:
        yf_list_reset(&node->as.call.args);
        for (;;) {
            if (yf_list_get(&node->as.call.args, (void **) &anode) == -1)
                break;
            if (anode)
                yf_cleanup_anode(anode, 1);
            yf_list_next(&node->as.call.args);
        }
        yf_list_destroy(&node->as.call.args);
    }
}

void yf_cleanup_avardecl(struct yfa_vardecl * node) {
    if (node->expr)
        yf_cleanup_anode(node->expr, 1);
}

void yf_cleanup_afuncdecl(struct yfa_funcdecl * node) {
    struct yfa_vardecl * vardecl;
    yf_list_reset(&node->params);
    for (;;) {
        if (yf_list_get(&node->params, (void **) &vardecl) == -1)
            break;
        yf_cleanup_avardecl(vardecl);
        yf_free(vardecl);
        yf_list_next(&node->params);
    }
    yf_list_destroy(&node->params);
    if (node->body)
        yf_cleanup_anode(node->body, 1);
}

void yf_cleanup_aprogram(struct yfa_program * node) {
    struct yf_ast_node * decl;
    yf_list_reset(&node->decls);
    for (;;) {
        if (yf_list_get(&node->decls, (void **) &decl) == -1)
            break;
        if (decl)
            yf_cleanup_anode(decl, 1);
        yf_list_next(&node->decls);
    }
    yf_list_destroy(&node->decls);
}

void yf_cleanup_abstmt(struct yfa_bstmt * node) {
    struct yf_ast_node * stmt;
    yf_list_reset(&node->stmts);
    for (;;) {
        if (yf_list_get(&node->stmts, (void **) &stmt) == -1)
            break;
        if (stmt)
            yf_cleanup_anode(stmt, 1);
        yf_list_next(&node->stmts);
    }
    yf_list_destroy(&node->stmts);
}

void yf_cleanup_areturn(struct yfa_return * node) {
    if (node->expr)
        yf_cleanup_anode(node->expr, 1);
}

void yf_cleanup_aif(struct yfa_if * node) {
    yf_cleanup_anode(node->cond, 1);
    yf_cleanup_anode(node->code, 1);
    if (node->elsebranch)
        yf_cleanup_anode(node->elsebranch, 1);
}

void yf_cleanup_ast(struct yf_ast_node * node) {
    yf_cleanup_anode(node, 0);
}

void yf_cleanup_cexpr(struct yfcs_expr * node);
void yf_cleanup_cvardecl(struct yfcs_vardecl * node);
void yf_cleanup_cfuncdecl(struct yfcs_funcdecl * node);
void yf_cleanup_cprogram(struct yfcs_program * node);
void yf_cleanup_cbstmt(struct yfcs_bstmt * node);
void yf_cleanup_creturn(struct yfcs_return * node);
void yf_cleanup_cif(struct yfcs_if * node);

void yf_cleanup_cnode(struct yf_parse_node * node, int free_node) {
    switch (node->type) {
    case YFCS_EMPTY:
        break;
    case YFCS_EXPR:
        yf_cleanup_cexpr(&node->expr);
        break;
    case YFCS_VARDECL:
        yf_cleanup_cvardecl(&node->vardecl);
        break;
    case YFCS_FUNCDECL:
        yf_cleanup_cfuncdecl(&node->funcdecl);
        break;
    case YFCS_PROGRAM:
        yf_cleanup_cprogram(&node->program);
        break;
    case YFCS_BSTMT:
        yf_cleanup_cbstmt(&node->bstmt);
        break;
    case YFCS_RET:
        yf_cleanup_creturn(&node->ret);
        break;
    case YFCS_IF:
        yf_cleanup_cif(&node->ifstmt);
        break;
    }
    if (free_node)
        yf_free(node);
}

void yf_cleanup_cexpr(struct yfcs_expr * node) {
    struct yf_parse_node * cnode;
    switch (node->type) {
    case YFCS_VALUE:
        break;
    case YFCS_BINARY:
        yf_cleanup_cnode(node->binary.left, 1);
        yf_free(node->binary.left);
        yf_cleanup_cnode(node->binary.right, 1);
        yf_free(node->binary.right);
        break;
    case YFA_FUNCCALL:
        yf_list_reset(&node->call.args);
        for (;;) {
            if (yf_list_get(&node->call.args, (void **) &cnode) == -1)
                break;
            if (cnode)
                yf_cleanup_cnode(cnode, 1);
            yf_list_next(&node->call.args);
        }
        yf_list_destroy(&node->call.args);
    }
}

void yf_cleanup_cvardecl(struct yfcs_vardecl * node) {
    if (node->expr)
        yf_cleanup_cnode(node->expr, 1);
}

void yf_cleanup_cfuncdecl(struct yfcs_funcdecl * node) {
    struct yfcs_vardecl * vardecl;
    yf_list_reset(&node->params);
    for (;;) {
        if (yf_list_get(&node->params, (void **) &vardecl) == -1)
            break;
        yf_cleanup_cvardecl(vardecl);
        yf_free(vardecl);
        yf_list_next(&node->params);
    }
    yf_list_destroy(&node->params);
    if (node->body)
        yf_cleanup_cnode(node->body, 1);
}

void yf_cleanup_cprogram(struct yfcs_program * node) {
    struct yf_parse_node * decl;
    yf_list_reset(&node->decls);
    for (;;) {
        if (yf_list_get(&node->decls, (void **) &decl) == -1)
            break;
        if (decl)
            yf_cleanup_cnode(decl, 1);
        yf_list_next(&node->decls);
    }
    yf_list_destroy(&node->decls);
}

void yf_cleanup_cbstmt(struct yfcs_bstmt * node) {
    struct yf_parse_node * stmt;
    yf_list_reset(&node->stmts);
    for (;;) {
        if (yf_list_get(&node->stmts, (void **) &stmt) == -1)
            break;
        if (stmt)
            yf_cleanup_cnode(stmt, 1);
        yf_list_next(&node->stmts);
    }
    yf_list_destroy(&node->stmts);
}

void yf_cleanup_creturn(struct yfcs_return * node) {
    if (node->expr)
        yf_cleanup_cnode(node->expr, 1);
}

void yf_cleanup_cif(struct yfcs_if * node) {
    yf_cleanup_cnode(node->cond, 1);
    yf_cleanup_cnode(node->code, 1);
    if (node->elsebranch)
        yf_cleanup_cnode(node->elsebranch, 1);
}

void yf_cleanup_cst(struct yf_parse_node * node) {
    yf_cleanup_cnode(node, 0);
}
