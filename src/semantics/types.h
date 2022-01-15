/**
 * Code for determining whether casts are valid, parsing literals, etc.
 */

#ifndef SEMANTICS_TYPES_H
#define SEMANTICS_TYPES_H

#include <stdbool.h>

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

#endif /* SEMANTICS_TYPES_H */
