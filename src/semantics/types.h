/**
 * Code for determining whether casts are valid, parsing literals, etc.
 */

#ifndef SEMANTICS_TYPES_H
#define SEMANTICS_TYPES_H

#include <stdbool.h>

#include <api/abstract-tree.h>
#include <semantics/validate-utils.h>

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
    struct yfa_expr * expr, struct yf_file_compilation_data * fdata
);

/**
 * Output any warnings and errors about whether conversion is OK.
 * Return: 0 if OK / warning, 1 if error.
 */
int yfs_output_diagnostics(
    struct yfs_type *, struct yfs_type *, struct yf_file_compilation_data *,
    int lineno
);

#endif /* SEMANTICS_TYPES_H */
