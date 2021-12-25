/**
 * A hashmap. This will primarily be used for storing symbols in a symbol
 * table, although further uses may be possible.
 */

#ifndef UTIL_HASHMAP_H
#define UTIL_HASHMAP_H

#include <stdbool.h>

/**
 * A hashmap, with string keys and void pointers as values. Values may NOT be
 * NULL, as this indicates an empty value bucket.
 */
struct yf_hashmap;

/* Create a new hashmap and return a pointer to it. */
struct yf_hashmap * yfh_new();

/* Destroy a hashmap. The "sub" argument is whether or not to free the pointers
stored as hashmap values. */
void yfh_destroy(struct yf_hashmap *, bool sub);

/**
 * Return 1 if adding a value failed.
 */
int yfh_set(struct yf_hashmap *, char * key, void * value);

/**
 * Returns NULL if no key is set.
 */
void * yfh_get(struct yf_hashmap *, char * key);

#endif /* UTIL_HASHMAP_H */
