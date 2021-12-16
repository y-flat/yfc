/**
 * An allocator. Mostly a wrapper around malloc for now, but may be more
 * complicated later. This mostly exists for the purpose of printing out
 * errors.
 */

#ifndef UTIL_ALLOCATOR_H
#define UTIL_ALLOCATOR_H

#include <stddef.h>
#include <stdlib.h>

void * yf_malloc(size_t size);
void yf_free(void * ptr);

#endif /* UTIL_ALLOCATOR_H */
