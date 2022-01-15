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
