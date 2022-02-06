/**
 * All "cleanup" functions - for freeing up unused API structures.
 */

#include <api/abstract-tree.h>
#include <util/allocator.h>

void yf_cleanup_expr(struct yfa_expr * node);
void yf_cleanup_vardecl(struct yfa_vardecl * node);
void yf_cleanup_funcdecl(struct yfa_funcdecl * node);
void yf_cleanup_program(struct yfa_program * node);
void yf_cleanup_bstmt(struct yfa_bstmt * node);
void yf_cleanup_return(struct yfa_return * node);
void yf_cleanup_if(struct yfa_if * node);

void yf_cleanup_node(struct yf_ast_node * node, int free_node) {
    switch (node->type) {
    case YFA_EMPTY:
        break;
    case YFA_EXPR:
        yf_cleanup_expr(&node->expr);
        break;
    case YFA_VARDECL:
        yf_cleanup_vardecl(&node->vardecl);
        break;
    case YFA_FUNCDECL:
        yf_cleanup_funcdecl(&node->funcdecl);
        break;
    case YFA_PROGRAM:
        yf_cleanup_program(&node->program);
        break;
    case YFA_BSTMT:
        yf_cleanup_bstmt(&node->bstmt);
        break;
    case YFA_RETURN:
        yf_cleanup_return(&node->ret);
        break;
    case YFA_IF:
        yf_cleanup_if(&node->ifstmt);
        break;
    }
    if (free_node)
        yf_free(node);
}

void yf_cleanup_expr(struct yfa_expr * node) {
    struct yf_ast_node * anode;
    switch (node->type) {
    case YFA_VALUE:
        break;
    case YFA_BINARY:
        yf_cleanup_expr(node->as.binary.left);
        yf_free(node->as.binary.left);
        yf_cleanup_expr(node->as.binary.right);
        yf_free(node->as.binary.right);
        break;
    case YFA_FUNCCALL:
        yf_list_reset(&node->as.call.args);
        for (;;) {
            if (yf_list_get(&node->as.call.args, (void **) &anode) == -1)
                break;
            if (anode)
                yf_cleanup_node(anode, 1);
            yf_list_next(&node->as.call.args);
        }
        yf_list_destroy(&node->as.call.args);
    }
}

void yf_cleanup_vardecl(struct yfa_vardecl * node) {
    if (node->expr)
        yf_cleanup_node(node->expr, 1);
}

void yf_cleanup_funcdecl(struct yfa_funcdecl * node) {
    struct yfa_vardecl * vardecl;
    yf_list_reset(&node->params);
    for (;;) {
        if (yf_list_get(&node->params, (void **) &vardecl) == -1)
            break;
        yf_cleanup_vardecl(vardecl);
        yf_free(vardecl);
        yf_list_next(&node->params);
    }
    yf_list_destroy(&node->params);
    if (node->body)
        yf_cleanup_node(node->body, 1);
}

void yf_cleanup_program(struct yfa_program * node) {
    struct yf_ast_node * decl;
    yf_list_reset(&node->decls);
    for (;;) {
        if (yf_list_get(&node->decls, (void **) &decl) == -1)
            break;
        if (decl)
            yf_cleanup_node(decl, 1);
        yf_list_next(&node->decls);
    }
    yf_list_destroy(&node->decls);
}

void yf_cleanup_bstmt(struct yfa_bstmt * node) {
    struct yf_ast_node * stmt;
    yf_list_reset(&node->stmts);
    for (;;) {
        if (yf_list_get(&node->stmts, (void **) &stmt) == -1)
            break;
        if (stmt)
            yf_cleanup_node(stmt, 1);
        yf_list_next(&node->stmts);
    }
    yf_list_destroy(&node->stmts);
}

void yf_cleanup_return(struct yfa_return * node) {
    if (node->expr)
        yf_cleanup_node(node->expr, 1);
}

void yf_cleanup_if(struct yfa_if * node) {
    yf_cleanup_node(node->cond, 1);
    yf_cleanup_node(node->code, 1);
    if (node->elsebranch)
        yf_cleanup_node(node->elsebranch, 1);
}

void yf_cleanup_ast(struct yf_ast_node * node) {
    yf_cleanup_node(node, 0);
}
