#include "symgen.h"

#include <util/yfc-out.h>

static int dump_symbol(
    FILE * out,
    struct yf_sym * sym
) {
    /* TODO */
    return 0;
}

int yfg_output_syms(
    struct yf_compilation_data * cdata,
    struct yf_compile_analyse_job * data,
    FILE * out
) {

    struct yfh_cursor cursor;
    struct yfs_symtab * fsymtab;

    /* We iterate through all symtabs and dump all decls into the file */
    for (yfh_cursor_init(&cursor, &cdata->symtables); !yfh_cursor_next(&cursor); ) {

        yfh_cursor_get(&cursor, NULL, (void **)&fsymtab);

        struct yfh_cursor inner_cursor;
        struct yf_sym * sym;
        int fail;

        for (yfh_cursor_init(&inner_cursor, &fsymtab->table); !yfh_cursor_next(&cursor); ) {
            yfh_cursor_get(&inner_cursor, NULL, (void **)&sym);
            if ((fail = dump_symbol(out, sym))) {
                YF_PRINT_ERROR("gen: panic: error in dumping symbol");
                return fail;
            }
        }

    }

    return 0;

}
