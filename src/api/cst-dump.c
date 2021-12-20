#include "cst-dump.h"

#include <stdio.h>

/* Forwards */
static void yf_dump_program(struct yfcs_program * node, FILE * out);
static void yf_dump_vardecl(struct yfcs_vardecl * node, FILE * out);
static void yf_dump_funcdecl(struct yfcs_funcdecl * node, FILE * out);
static void yf_dump_expr(struct yfcs_expr * node, FILE * out);

/* TODO - non-static this, maybe by puting in a struct or something. */
static int yf_dump_indent = 0;

static void indent() { ++yf_dump_indent; }
static void dedent() { --yf_dump_indent; }

void yf_dump_cst(struct yf_parse_node * root, FILE *out) {

    switch (root->type) {
        case YFCS_PROGRAM:
            yf_dump_program(&root->as.program, out);
            break;
        case YFCS_VARDECL:
            yf_dump_vardecl(&root->as.vardecl, out);
            break;
        case YFCS_FUNCDECL:
            yf_dump_funcdecl(&root->as.funcdecl, out);
            break;
        case YFCS_EXPR:
            yf_dump_expr(&root->as.expr, out);
            break;
    }

}

static void yf_print_line(FILE * out, char * data) {
    int i;
    for (i = 0; i < yf_dump_indent; i++) {
        fprintf(out, "\t");
    }
    fprintf(out, "%s\n", data);
}

static void yf_dump_program(struct yfcs_program * node, FILE *out) {

    struct yf_parse_node * child;

    yf_print_line(out, "program");
    indent();
    for (;;) {
        yf_list_get(&node->decls, (void **) &child);
        if (!child) break;
        yf_dump_cst(child, out);
    }
    dedent();

    yf_list_reset(&node->decls);

    yf_print_line(out, "end program");

}

static void yf_dump_vardecl(struct yfcs_vardecl * node, FILE * out) {
    /* TODO */
    yf_print_line(out, "vardecl");
    yf_print_line(out, "end vardecl");
}

static void yf_dump_funcdecl(struct yfcs_funcdecl * node, FILE * out) {
    /* TODO */
    yf_print_line(out, "funcdecl");
    yf_print_line(out, "end funcdecl");
}

static void yf_dump_expr(struct yfcs_expr * node, FILE * out) {
    /* TODO */
    yf_print_line(out, "expr");
    yf_print_line(out, "end expr");
}
