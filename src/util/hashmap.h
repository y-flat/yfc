/**
 * A hashmap. This will primarily be used for storing symbols in a symbol
 * table, although further uses may be possible.
 */

#ifndef UTIL_HASHMAP_H
#define UTIL_HASHMAP_H

#include <stdbool.h>

/**
 * Perhaps larger in the future
 */
#define YFH_BUCKETS 100000

/**
 * A hashmap, with string keys and void pointers as values. Values may NOT be
 * NULL, as this indicates an empty value bucket.
 *
 * A bunch of buckets with values. Buckets also store the key so that collisions
 * can be avoided, by simply moving the value to be inserted into the next free
 * bucket.
 */
struct yf_hashmap {

    struct yfh_bucket {
        const char * key;
        void * value;
    } * buckets;

};

/* Create a new hashmap and return a pointer to it. */
struct yf_hashmap * yfh_new();

/**
 * Destroy a hashmap.
 * The cleanup hook is a function used to free the values stored in the hashmap,
 * if cleanup is NULL then no cleanup takes places.
 */
void yfh_destroy(struct yf_hashmap *, int (*cleanup)(void *));

/**
 * Return 1 if adding a value failed.
 */
int yfh_set(struct yf_hashmap *, const char * key, void * value);

/**
 * Returns NULL if no key is set.
 */
void * yfh_get(struct yf_hashmap *, char * key);

#endif /* UTIL_HASHMAP_H */
