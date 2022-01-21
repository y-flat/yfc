#include "typegen.h"

#include <stdio.h>

#include <util/yfc-out.h>

int yfg_ctype(int len, char * buf, struct yfs_type * type) {

    if (type->kind != YFST_PRIMITIVE) {
        return -1; /* No can do */
    }

    if (type->primitive.size >= 8) {
        /* Because stdint is #include'd, we can just print int[blank]_t. */
        if (type->primitive.type == YFS_INT) {
            snprintf(
                buf, len, "int%d_t",
                type->primitive.size
            );
        } else {
            /* It's a float. */
            switch (type->primitive.size) {
                case 16:
                case 32:
                    snprintf(buf, len, "float");
                    break;
                case 64:
                    snprintf(buf, len, "double");
                    break;
                default:
                    YF_PRINT_ERROR(
                        "Unsupported float size: %d",
                        type->primitive.size
                    );
                    return -1;
            }
        }
    } else {
        /* Only < 8 size is bool. */
        if (type->primitive.size)
            snprintf(buf, len, "_Bool");
        else
            snprintf(buf, len, "void");
    }

    return 0;

}
