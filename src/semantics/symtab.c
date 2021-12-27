#include "symtab.h"

#include <api/sym.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

static int yfs_add_var(struct yf_hashmap * symtab, struct yfcs_vardecl *);
static int yfs_add_fn(struct yf_hashmap * symtab, struct yfcs_funcdecl *);

int yfs_build_symtab(struct yf_file_compilation_data * data) {

    struct yf_parse_node * node;
    int ret;

    data->symtab.table = yfh_new();
    if (!data->symtab.table) {
        YF_PRINT_ERROR("symtab: failed to allocate table");
        return 3; /* Memory error */
    }

    /* Get to beginning of list traversal */
    yf_list_reset(&data->parse_tree.as.program.decls);
    
    ret = 0;
    for (;;) {
        if (yf_list_get(
            &data->parse_tree.as.program.decls,
            (void **) &node
        ) == -1)
            return ret;
        switch (node->type) {
            case YFCS_VARDECL:
                if (yfs_add_var(data->symtab.table, &node->as.vardecl))
                    ret = 1;
                break;
            case YFCS_FUNCDECL:
                if (yfs_add_fn(data->symtab.table, &node->as.funcdecl))
                    ret = 1;
                break;
            default:
                YF_PRINT_ERROR("symtab: panic: unrecognized global node type");
                return 2; /* Internal error */
        }
        yf_list_next(&data->parse_tree.as.program.decls);
    }

    return ret;

}

static int yfs_add_var(struct yf_hashmap * symtab, struct yfcs_vardecl * v) {

    struct yf_sym * vsym;
    vsym = yf_malloc(sizeof (struct yf_sym));
    if (!vsym) return 3;

    vsym->type = YFS_VAR;
    
    /* TODO */
    vsym->line = 0;
    vsym->file = "<unknown>";
    
    vsym->var.name = v->name.name.databuf;
    vsym->var.dtype.name = v->type.databuf;

    return 0;

}

static int yfs_add_fn(struct yf_hashmap * symtab, struct yfcs_funcdecl * f) {
    YF_PRINT_ERROR("function symbol creation not supported yet");
    return 1;
}
