#include "gen.h"

#include <stdarg.h>
#include <stdio.h>

#include <api/abstract-tree.h>
#include <api/operator.h>
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

static void yfg_print_line(FILE * out, char * data, ...) {
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
        if (child->type == YFA_VARDECL)
            yfg_print_line(out, ";");
        else
            yfg_print_line(out, "");
        yf_list_next(&node->decls);
    }

    yf_list_reset(&node->decls);

}

static void yf_gen_vardecl(struct yfa_vardecl * node, FILE * out) {
    fprintf(out, "%s %s", node->name->var.dtype->name, node->name->var.name);
    if (node->expr) {
        fprintf(out, " = ");
        yf_gen_node(node->expr, out);
    }
}

static void yf_gen_funcdecl(struct yfa_funcdecl * node, FILE * out) {

    struct yf_ast_node * child;
    int argct = 0;

    fprintf(out, "%s %s", node->name->fn.rtype->name, node->name->fn.name);
    fprintf(out, "(");

    /* Generate param list */

    yf_list_reset(&node->params);

    for (;;) {
        if (argct)
            fprintf(out, ", ");
        if (yf_list_get(&node->params, (void **) &child) == -1) break;
        if (!child) break;
        yf_gen_node(child, out);
        yf_list_next(&node->params);
        ++argct;
    }

    fprintf(out, ")");

    yf_gen_node(node->body, out);

}

static void yf_gen_expr(struct yfa_expr * node, FILE * out) {

    struct yf_ast_node * call_arg;
    int argct;

    /* All expressions are surrounded in parens so C's operator precedence
     * is ignored. */
    fprintf(out, "(");

    switch (node->type) {
        case YFA_VALUE:
            switch (node->as.value.type) {
                case YFA_LITERAL:
                    fprintf(out, "%d", node->as.value.as.literal.val);
                    break;
                case YFA_IDENT:
                    fprintf(out, "%s", node->as.value.as.identifier->var.name);
                    break;
            }
            break;
        case YFA_BINARY:
            yf_gen_expr(node->as.binary.left, out);
            fprintf(out, " %s ", get_op_string(node->as.binary.op));
            yf_gen_expr(node->as.binary.right, out);
            break;
        case YFA_FUNCCALL:
            fprintf(out, "%s(", node->as.call.name->fn.name);
            yf_list_reset(&node->as.call.args);
            argct = 0;
            for (;;) {
                if (yf_list_get(&node->as.call.args, (void **) &call_arg) == -1) break;
                if (!call_arg) break;
                yf_gen_node(call_arg, out);
                yf_list_next(&node->as.call.args);
                if (argct)
                    fprintf(out, ", ");
                ++argct;
            }
            fprintf(out, ")");
            break;
    }

    fprintf(out, ")");

}

static void yf_gen_bstmt(struct yfa_bstmt * node, FILE * out) {
    struct yf_ast_node * child;
    fprintf(out, "{");
    indent();
    for (;;) {
        if (yf_list_get(&node->stmts, (void **) &child) == -1) break;
        if (!child) break;
        yf_gen_node(child, out);
        yfg_print_line(out, ";");
        yf_list_next(&node->stmts);
    }
    dedent();
    fprintf(out, "}");
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
