#include "cst-dump.h"

#include <stdarg.h>
#include <stdio.h>

/**
 * The CST-dumping routines are just for debugging purposes. Each node type has
 * a corresponding dump function (except for empty nodes, which just print out)
 * <empty statement>.
 */

/* Forwards */
static void yf_dump_program(struct yfcs_program * node, FILE * out);
static void yf_dump_vardecl(struct yfcs_vardecl * node, FILE * out);
static void yf_dump_funcdecl(struct yfcs_funcdecl * node, FILE * out);
static void yf_dump_expr(struct yfcs_expr * node, FILE * out);
static void yf_dump_bstmt(struct yfcs_bstmt * node, FILE * out);
static void yf_dump_ret(struct yfcs_return * node, FILE * out);
static void yf_dump_if(struct yfcs_if * node, FILE * out);

/**
 * TODO - non-static this, maybe by putting in a struct or something.
 * 
 * Because the dumped code is well-formatted (by some bare standards), code is
 * indented and that amount is stored here. As shown in yf_print_line, every new
 * line gets printed with a number of tabs equal to this variable.
 */
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

/**
 * Finds the underlying type of a tagged union and calls the appropriate
 * function.
 */
void yf_dump_cst(struct yf_parse_node * root, FILE *out) {

    if (root->loc.line && root->loc.column)
        yf_print_line(
            out, "line: %d, colno: %d",
            root->loc.line, root->loc.column
        );

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
            break;
        case YFCS_RET:
            yf_dump_ret(&root->ret, out);
            break;
        case YFCS_IF:
            yf_dump_if(&root->ifstmt, out);
            break;
        case YFCS_EMPTY:
            yf_print_line(out, "<empty statement>");
            break;
    }

}

static void yf_dump_program(struct yfcs_program * node, FILE *out) {

    struct yf_parse_node * child;

    yf_print_line(out, "program");
    indent();
    YF_LIST_FOREACH(node->decls, child) {
        yf_dump_cst(child, out);
    }
    dedent();

    yf_print_line(out, "end program");

}

static void yf_dump_vardecl(struct yfcs_vardecl * node, FILE * out) {

    yf_print_line(out, "vardecl");
    indent();

    yf_print_line(out, "name: %s::%s", node->name.filepath, node->name.name);
    yf_print_line(out, "type: %s", node->type.databuf);

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
    yf_print_line(out, "params");
    indent();
    struct yf_parse_node * param;
    YF_LIST_FOREACH(node->params, param) {
        yf_dump_cst(param, out);
    }
    dedent();
    yf_print_line(out, "end params");
    yf_print_line(out, "return type: %s", node->ret.databuf);
    yf_print_line(out, "function body");
    indent();
    if (node->body) {
        yf_dump_cst(node->body, out);
    } else {
        yf_print_line(out, "[no body]");
    }
    dedent();
    yf_print_line(out, "end function body");
    dedent();
    yf_print_line(out, "end funcdecl");
}

static void yf_dump_expr(struct yfcs_expr * node, FILE * out) {

    struct yf_parse_node * arg;

    yf_print_line(out, "expr");
    indent();

    switch (node->type) {
    case YFCS_E_VALUE:
        if (node->value.type != YFCS_V_IDENT)
            yf_print_line(out, "value: %s",
                node->value.literal.value
            );
        else
            yf_print_line(out, "identifier: %s::%s",
                node->value.identifier.filepath, node->value.identifier.name
            );
        break;
    case YFCS_E_BINARY:
        yf_print_line(out,
            "operator: %s", get_op_string(node->binary.op)
        );
        yf_print_line(out, "left:");
        yf_dump_expr(&node->binary.left->expr,  out);
        yf_print_line(out, "right:");
        yf_dump_expr(&node->binary.right->expr, out);
        break;
    case YFCS_E_FUNCCALL:
        yf_print_line(
            out, "function name: %s::%s",
            node->call.name.filepath, node->call.name.name
        );
        yf_print_line(out, "arguments:");
        indent();
        YF_LIST_FOREACH(node->call.args, arg) {
            yf_dump_cst(arg, out);
        }
        dedent();
        yf_print_line(out, "end arguments");
        break;
    }

    dedent();
    yf_print_line(out, "end expr");

}

static void yf_dump_bstmt(struct yfcs_bstmt * node, FILE * out) {
    struct yf_parse_node * child;
    yf_print_line(out, "block statement");
    indent();
    YF_LIST_FOREACH(node->stmts, child) {
        yf_dump_cst(child, out);
    }
    dedent();
    yf_print_line(out, "end block statement");
}

static void yf_dump_ret(struct yfcs_return * node, FILE * out) {
    yf_print_line(out, "return");
    indent();
    if (node->expr)
        yf_dump_expr(&node->expr->expr, out);
    dedent();
    yf_print_line(out, "end return");
}

static void yf_dump_if(struct yfcs_if * node, FILE * out) {
    yf_print_line(out, "if");
    yf_print_line(out, "condition");
    indent();
    yf_dump_expr(&node->cond->expr, out);
    dedent();
    yf_print_line(out, "end condition");
    yf_print_line(out, "then");
    indent();
    yf_dump_cst(node->cond, out);
    dedent();
    if (node->elsebranch != NULL) {
        yf_print_line(out, "else");
        indent();
        yf_dump_cst(node->elsebranch, out);
        dedent();
    }
    yf_print_line(out, "end if");
}
