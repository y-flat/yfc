/**
 * Code for determining whether casts are valid, parsing literals, etc.
 */

#ifndef SEMANTICS_TYPES_H
#define SEMANTICS_TYPES_H

#include <stdbool.h>

#include <api/abstract-tree.h>
#include <api/loc.h>
#include <semantics/validate/validate-internal.h>

void yft_add_type(
    struct yf_compile_analyse_job * udata,
    char * name, int size, enum yfpt_format fmt
);

void yfv_add_builtin_types(struct yf_compile_analyse_job * udata);

enum yfs_conversion_allowedness {
    YFS_CONVERSION_OK,
    YFS_CONVERSION_LOSSY, /* like i64 -> bool */
    YFS_CONVERSION_INVALID, /* like class MyType -> int */
    YFS_CONVERSION_VOID, /* anything involving void */
};

/**
 * Whether the first type can be converted to the second without any possible
 * information loss.
 */
enum yfs_conversion_allowedness yfs_is_safe_conversion(
    struct yfs_type * from, struct yfs_type * to
);

const char * yfse_get_error_message(enum yfs_conversion_allowedness err);

struct yfs_type * yfse_get_expr_type(
    struct yfa_expr * expr, struct yf_compile_analyse_job * udata
);

/**
 * Output any warnings and errors about whether conversion is OK.
 * Return: 0 if OK / warning, 1 if error.
 */
int yfs_output_diagnostics(
    struct yfs_type *, struct yfs_type *, struct yf_compile_analyse_job *,
    struct yf_location * loc
);

#endif /* SEMANTICS_TYPES_H */
