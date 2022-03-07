#include "types.h"

enum yfs_conversion_allowedness yfs_is_safe_conversion(
    struct yfs_type * from, struct yfs_type * to
) {

    struct yfs_primitive_type * f, * t;

    /* TODO - get user-defined conversion operators (should they exist) */
    if (from->kind != to->kind) {
        /* One is a primitive, the other isn't. Whoops. */
        return YFS_CONVERSION_INVALID;
    }

    /* User-defined types don't exist yet, and once they do, conversion won't
    exist (for now). */
    if (from->kind != YFST_PRIMITIVE)
        return YFS_CONVERSION_INVALID;

    f = &from->primitive;
    t = &to->primitive;

    if (f->size == 0 || t->size == 0)
        return YFS_CONVERSION_VOID;

    if (f->size > t->size || f->type != t->type) {
        return YFS_CONVERSION_LOSSY;
    }

    return YFS_CONVERSION_OK;

}

const char * yfse_get_error_message(enum yfs_conversion_allowedness err) {
    static const char * errs[] = {
        "No error",
        "Potential loss of data",
        "Invalid conversion",
        "Conversion to or from void",
    };
    return errs[err];
}

struct yfs_type * yfse_get_expr_type(
    struct yfa_expr * expr, struct yf_compile_analyse_job * fdata
) {

    /* This function is super hacky and whatnot. */

    int lsize, rsize;
    struct yfs_type * ltype, * rtype;
    struct yfa_value * v;

    switch (expr->type) {
    case YFA_BINARY:

        ltype = yfse_get_expr_type(expr->as.binary.left, fdata);
        rtype = yfse_get_expr_type(expr->as.binary.right, fdata);
        lsize = ltype->primitive.size;
        rsize = rtype->primitive.size;

        if (yfo_is_bool(expr->as.binary.op)) {
            return yfv_get_type_s(fdata, "bool");
        }

        if (yfo_is_assign(expr->as.binary.op)) {
            return ltype;
        }

        /* By default, ... */
        /* Return the larger size. TODO - make this better. */
        if (lsize > rsize) {
            return ltype;
        } else {
            return rtype;
        }
        break;

    case YFA_VALUE:
        v = &expr->as.value;
        if (v->type == YFA_IDENT) {
            return v->as.identifier->var.dtype;
        } else {
            switch (v->as.literal.type) {
            case YFAL_NUM:
                return yfv_get_type_s(fdata, "int");
            case YFAL_BOOL:
                return yfv_get_type_s(fdata, "bool");
            default:
                YF_PRINT_ERROR("panic: Unknown literal type");
                return NULL;
            }
        }
        break;
    case YFA_FUNCCALL:
        return expr->as.call.name->fn.rtype;
    }

}

int yfs_output_diagnostics(
    struct yfs_type * from,
    struct yfs_type * to,
    struct yf_compile_analyse_job * fdata,
    struct yf_location * loc
) {

    enum yfs_conversion_allowedness al;

    if ( (al = yfs_is_safe_conversion(
            from, to
        )) != YFS_CONVERSION_OK
    ) {
        /* TODO - reduce repetition */
        if (al == YFS_CONVERSION_LOSSY) {
            YF_PRINT_WARNING("%s %d:%d: %s when converting from %s to %s",
                loc->file,
                loc->line,
                loc->column,
                yfse_get_error_message(al),
                from->name,
                to->name
            );
        } else {
            YF_PRINT_ERROR("%s %d:%d: %s when converting from %s to %s",
                loc->file,
                loc->line,
                loc->column,
                yfse_get_error_message(al),
                from->name,
                to->name
            );
            return 1;
        }
    }
    return 0;
}
