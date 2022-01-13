#include "symtab.h"

#include <api/sym.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

static int yfs_add_var(struct yf_hashmap * symtab, struct yf_parse_node *);
static int yfs_add_fn(struct yf_hashmap * symtab, struct yf_parse_node *);

int yfs_build_symtab(struct yf_file_compilation_data * data) {

    struct yf_parse_node * node;
    int ret;

    data->symtab.table = yfh_new();
    data->symtab.parent = NULL;
    if (!data->symtab.table) {
        YF_PRINT_ERROR("symtab: failed to allocate table");
        return 3; /* Memory error */
    }

    /* Get to beginning of list traversal */
    yf_list_reset(&data->parse_tree.program.decls);
    
    ret = 0;
    for (;;) {
        if (yf_list_get(
            &data->parse_tree.program.decls,
            (void **) &node
        ) == -1)
            return ret;
        switch (node->type) {
            case YFCS_VARDECL:
                if (yfs_add_var(data->symtab.table, node))
                    ret = 1;
                break;
            case YFCS_FUNCDECL:
                if (yfs_add_fn(data->symtab.table, node))
                    ret = 1;
                break;
            default:
                YF_PRINT_ERROR("symtab: panic: unrecognized global node type");
                return 2; /* Internal error */
        }
        yf_list_next(&data->parse_tree.program.decls);
    }

    return ret;

}

static int yfs_add_var(struct yf_hashmap * symtab, struct yf_parse_node * n) {

    struct yfcs_vardecl * v = &n->vardecl;
    struct yf_sym * vsym, * dupl;
    vsym = yf_malloc(sizeof (struct yf_sym));
    if (!vsym) return 3;

    vsym->type = YFS_VAR;
    
    vsym->line = n->lineno;
    /* TODO */
    vsym->file = "<unknown>";
    
    vsym->var.name = v->name.name;

    if ( (dupl = yfh_get(symtab, vsym->var.name)) != NULL) {
        YF_PRINT_ERROR(
            "symtab: duplicate variable declaration '%s' (lines %d and %d)",
            v->name.name, dupl->line, vsym->line
        );
        free(vsym);
        return 1;
    }

    yfh_set(symtab, v->name.name, vsym);

    return 0;

}

static int yfs_add_fn(struct yf_hashmap * symtab, struct yf_parse_node * f) {
    
    struct yfcs_funcdecl * fn = &f->funcdecl;

    struct yf_sym * fsym;
    struct yf_parse_node * narg;
    struct yfcs_vardecl * arg;
    struct yfsn_param * param;

    fsym = yf_malloc(sizeof (struct yf_sym));
    if (!fsym)
        return 3;

    fsym->type = YFS_FN;
    fsym->line = f->lineno;
    /* Also TODO */
    fsym->file = "<unknown>";

    fsym->fn.name = fn->name.name;

    yf_list_init(&fsym->fn.params);
    yf_list_reset(&fn->params);

    /* Adding parameters to symbol */
    for (;;) {

        if (yf_list_get(
            &fn->params,
            (void **) &narg
        ) == -1)
            break;

        arg = &narg->vardecl;
        
        param = yf_malloc(sizeof (struct yfsn_param));
        if (!param) return 3;

        param->name = arg->name.name;
        param->type = arg->type.databuf;
        yf_list_add(&fsym->fn.params, param);

        yf_list_next(&fn->params);

    }

    YF_PRINT_ERROR("1");
    yfh_set(symtab, fsym->fn.name, fsym);
    YF_PRINT_ERROR("2");

    return 0;

}
