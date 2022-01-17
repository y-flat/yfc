#include "gen.h"

#include <stdarg.h>
#include <stdio.h>

#include <api/abstract-tree.h>
#include <util/yfc-out.h>

static void yf_gen_program(struct yfa_program * node, FILE * out);
static void yf_gen_vardecl(struct yfa_vardecl * node, FILE * out);
static void yf_gen_funcdecl(struct yfa_funcdecl * node, FILE * out);
static void yf_gen_expr(struct yfa_expr * node, FILE * out);
static void yf_gen_bstmt(struct yfa_bstmt * node, FILE * out);

/* TODO - non-static this, maybe by putting in a struct or something. */
static int yf_dump_indent = 0;

static void indent() { ++yf_dump_indent; }
static void dedent() { --yf_dump_indent; }

static void yf_print_line(FILE * out, char * data, ...) {
    int i;
    va_list args;
    va_start(args, data);
    for (i = 0; i < yf_dump_indent; i++) {
        fprintf(out, "\t");
    }
    vfprintf(out, data, args);
    fprintf(out, "\n");
    va_end(args);
}

void yf_gen_node(struct yf_ast_node * root, FILE *out) {

    switch (root->type) {
        case YFA_PROGRAM:
            yf_gen_program(&root->program, out);
            break;
        case YFA_VARDECL:
            yf_gen_vardecl(&root->vardecl, out);
            break;
        case YFA_FUNCDECL:
            yf_gen_funcdecl(&root->funcdecl, out);
            break;
        case YFA_EXPR:
            yf_gen_expr(&root->expr, out);
            break;
        case YFA_BSTMT:
            yf_gen_bstmt(&root->bstmt, out);
    }

}

static void yf_gen_program(struct yfa_program * node, FILE *out) {

    struct yf_ast_node * child;

    for (;;) {
        if (yf_list_get(&node->decls, (void **) &child) == -1) break;
        if (!child) break;
        yf_gen_node(child, out);
        yf_list_next(&node->decls);
    }

    yf_list_reset(&node->decls);

}

static void yf_gen_vardecl(struct yfa_vardecl * node, FILE * out) {
    /* TODO */
}

static void yf_gen_funcdecl(struct yfa_funcdecl * node, FILE * out) {
    /* TODO */
}

static void yf_gen_expr(struct yfa_expr * node, FILE * out) {
    /* TODO */
}

static void yf_gen_bstmt(struct yfa_bstmt * node, FILE * out) {
    struct yf_ast_node * child;
    indent();
    for (;;) {
        if (yf_list_get(&node->stmts, (void **) &child) == -1) break;
        if (!child) break;
        yf_gen_node(child, out);
        yf_list_next(&node->stmts);
    }
    dedent();
}

int yfg_gen(struct yf_file_compilation_data * data) {

    FILE * out;
    out = fopen(data->output_file, "w");
    if (!out) {
        YF_PRINT_ERROR("could not open output file %s", data->output_file);
        return 1;
    }

    yf_gen_node(&data->ast_tree, out);

    fclose(out);
    return 0;

}
