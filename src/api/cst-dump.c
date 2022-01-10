#include "cst-dump.h"

#include <stdarg.h>
#include <stdio.h>

/* Forwards */
static void yf_dump_program(struct yfcs_program * node, FILE * out);
static void yf_dump_vardecl(struct yfcs_vardecl * node, FILE * out);
static void yf_dump_funcdecl(struct yfcs_funcdecl * node, FILE * out);
static void yf_dump_expr(struct yfcs_expr * node, FILE * out);
static void yf_dump_bstmt(struct yfcs_bstmt * node, FILE * out);

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


void yf_dump_cst(struct yf_parse_node * root, FILE *out) {

    if (root->lineno && root->colno)
        yf_print_line(out, "line: %d, colno: %d", root->lineno, root->colno);

    switch (root->type) {
        case YFCS_PROGRAM:
            yf_dump_program(&root->program, out);
            break;
        case YFCS_VARDECL:
            yf_dump_vardecl(&root->vardecl, out);
            break;
        case YFCS_FUNCDECL:
            yf_dump_funcdecl(&root->funcdecl, out);
            break;
        case YFCS_EXPR:
            yf_dump_expr(&root->expr, out);
            break;
        case YFCS_BSTMT:
            yf_dump_bstmt(&root->bstmt, out);
    }

}

static void yf_dump_program(struct yfcs_program * node, FILE *out) {

    struct yf_parse_node * child;

    yf_print_line(out, "program");
    indent();
    for (;;) {
        if (yf_list_get(&node->decls, (void **) &child) == -1) break;
        if (!child) break;
        yf_dump_cst(child, out);
        yf_list_next(&node->decls);
    }
    dedent();

    yf_list_reset(&node->decls);

    yf_print_line(out, "end program");

}

static void yf_dump_vardecl(struct yfcs_vardecl * node, FILE * out) {

    yf_print_line(out, "vardecl");
    indent();

    yf_print_line(out, "name: %s", node->name.name);
    yf_print_line(out, "type: %s", node->type);

    yf_print_line(out, "initialization value:");
    if (node->expr) {
        yf_dump_cst(node->expr, out);
    } else {
        yf_print_line(out, "<none>");
    }

    dedent();
    yf_print_line(out, "end vardecl");

}

static void yf_dump_funcdecl(struct yfcs_funcdecl * node, FILE * out) {
    yf_print_line(out, "funcdecl");
    indent();
    /* TODO - args */
    yf_print_line(out, "function body");
    yf_dump_cst(node->body, out);
    yf_print_line(out, "end function body");
    dedent();
    yf_print_line(out, "end funcdecl");
}

static void yf_dump_expr(struct yfcs_expr * node, FILE * out) {

    yf_print_line(out, "expr");
    indent();

    if (node->type == YFCS_VALUE) {
        yf_print_line(out, "value: %s",
            node->value.type == YFCS_IDENT ? node->value.identifier.name
            : node->value.literal.value
        );
    } else {
        yf_print_line(out,
            "operator: %s", get_op_string(node->binary.op)
        );
        yf_print_line(out, "left:");
        yf_dump_expr(&node->binary.left->expr,  out);
        yf_print_line(out, "right:");
        yf_dump_expr(&node->binary.right->expr, out);
    }

    dedent();
    yf_print_line(out, "end expr");

}

static void yf_dump_bstmt(struct yfcs_bstmt * node, FILE * out) {
    struct yf_parse_node * child;
    yf_print_line(out, "block statement");
    indent();
    for (;;) {
        if (yf_list_get(&node->stmts, (void **) &child) == -1) break;
        if (!child) break;
        yf_dump_cst(child, out);
        yf_list_next(&node->stmts);
    }
    dedent();
    yf_print_line(out, "end block statement");
}
