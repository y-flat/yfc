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
/* Allocates an array of num_elems elements of a given size and zeroes the memory */
void * yf_calloc(size_t num_elems, size_t size);
void yf_free(void * ptr);

/**
 * A version of strcpy that returns pointer to the terminating NUL-byte for faster concatenations
 */
char * yf_strcpy(char * restrict dst, const char * src);

char * yf_strdup(const char * src);

#endif /* UTIL_ALLOCATOR_H */
