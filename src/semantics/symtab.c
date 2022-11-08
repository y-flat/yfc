#include "symtab.h"

#include <string.h>

#include <api/sym.h>
#include <semantics/types.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

static int yfs_add_var(struct yf_hashmap * symtab, struct yf_parse_node *);
static int yfs_add_fn(struct yf_hashmap * symtab, struct yf_parse_node *);

int yfs_build_symtab(struct yf_compile_analyse_job * data) {

    struct yf_parse_node * node;
    int ret;

    yfh_init(&data->symtab.table);
    data->symtab.parent = NULL;
    if (!data->symtab.table.buckets) {
        YF_PRINT_ERROR("symtab: failed to allocate table");
        return 3; /* Memory error */
    }

    yfh_init(&data->types.table);
    yfv_add_builtin_types(data);
    
    ret = 0;
    YF_LIST_FOREACH(data->parse_tree.program.decls, node) {
        switch (node->type) {
            case YFCS_VARDECL:
                if (yfs_add_var(&data->symtab.table, node))
                    ret = 1;
                break;
            case YFCS_FUNCDECL:
                if (yfs_add_fn(&data->symtab.table, node))
                    ret = 1;
                break;
            default:
                YF_PRINT_ERROR("symtab: panic: unrecognized global node type");
                return 2; /* Internal error */
        }
    }

    return ret;

}

static int yfs_add_var(struct yf_hashmap * symtab, struct yf_parse_node * n) {

    struct yfcs_vardecl * v = &n->vardecl;
    struct yf_sym * vsym, * dupl;
    vsym = yf_malloc(sizeof (struct yf_sym));
    if (!vsym) return 3;

    vsym->type = YFS_VAR;
    
    vsym->loc = n->loc;
    
    vsym->var.name = v->name.name;

    if (yfh_get(symtab, vsym->var.name, (void **)&dupl) == 0) {
        YF_PRINT_ERROR(
            "symtab: duplicate variable declaration '%s' (lines %d and %d)",
            v->name.name, dupl->loc.line, vsym->loc.line
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
    fsym->loc = f->loc;

    fsym->fn.name = fn->name.name;

    yf_list_init(&fsym->fn.params);

    /* Adding parameters to symbol */
    YF_LIST_FOREACH(fn->params, narg) {

        arg = &narg->vardecl;
        
        param = yf_malloc(sizeof (struct yfsn_param));
        if (!param) return 3;

        param->name = arg->name.name;
        param->type = arg->type.databuf;
        yf_list_add(&fsym->fn.params, param);

    }

    /* TODO -- actually look up custom types once those exist. */
    fsym->fn.rtype->kind = YFS_T_PRIMITIVE;
    strcpy(fsym->fn.rtype->name, fn->ret.databuf);

    yfh_set(symtab, fsym->fn.name, fsym);

    return 0;

}
