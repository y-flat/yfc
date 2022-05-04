/**
 * A hashmap. This will primarily be used for storing symbols in a symbol
 * table, although further uses may be possible.
 */

#ifndef UTIL_HASHMAP_H
#define UTIL_HASHMAP_H

#include "util/allocator.h"
#include <stdbool.h>

#define YFH_INIT_BUCKETS (0x1000 / sizeof(void*))

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
        char * key;
        void * value;
        struct yfh_bucket * next;
    } ** buckets;

    unsigned long num_buckets;

};

struct yfh_cursor {

    struct yf_hashmap * hashmap;
    struct yfh_bucket ** bucket;
    struct yfh_bucket * current;

};

/* Initialize a new hashmap. */
inline void yfh_init(struct yf_hashmap * map) {
    map->num_buckets = YFH_INIT_BUCKETS;
    map->buckets = yf_calloc(map->num_buckets, sizeof(struct yfh_bucket *));
}

/**
 * Destroy a hashmap.
 * The cleanup hook is a function used to free the values stored in the hashmap,
 * if cleanup is NULL then no cleanup takes places.
 * The cleanup function must not modify the hash map in any way.
 */
void yfh_destroy(struct yf_hashmap *, void (*cleanup)(void *));

/**
 * Tries to rehash the whole hashmap.
 * Not implemented yet!
 * Returns 0 on success, 1 on failure.
 * Regardless of the result, ALL existing cursors to the map will be invalidated and must optionally be re-initialised.
 * @param hint the number of buckets to use; 0 means rehash will pick an optimal number of buckets itself.
 */
int yfh_rehash(struct yf_hashmap *, unsigned hint);

/**
 * Return 1 if adding a value failed.
 * Probably will need to rehash on demand as well.
 */
int yfh_set(struct yf_hashmap *, const char * key, void * value);

/**
 * Return 1 if removing a value failed.
 * @param cleanup same as in yfh_destroy
 */
int yfh_remove(struct yf_hashmap *, const char * key, void (*cleanup)(void *));

/**
 * Return 1 if removing a value failed.
 * @param cur must be a valid cursor pointing to the element being removed.
 * After the operation, cursor points to the entry after the one removed or null if there's no one.
 * It should be recalibrated to retrieve the next element.
 * @param cleanup same as in yfh_destroy
 */
int yfh_remove_at(struct yfh_cursor * cur, void (*cleanup)(void *));

/**
 * Returns 1 if key was not present (value is unmodified), otherwise returns 0 and value is stored in value.
 */
int yfh_get(struct yf_hashmap *, const char * key, void ** value);

/**
 * Init/reset cursor to the beginning.
 * If hashmap is NULL, uses the last used hashmap in cursor, it is undefined behaviour if the cursor hasn't been used before.
 * The cursor must be 'next'ed or recalibrated before it can be dereferenced first.
 */
inline void yfh_cursor_init(struct yfh_cursor * cur, struct yf_hashmap * hashmap) {
    if (hashmap) {
        cur->hashmap = hashmap;
    }

    cur->bucket = NULL;
    cur->current = NULL;
}

/**
 * Returns 0 if cur points to a valid entry, and fills the output parameters. Returns 1 otherwise.
 * @param key key of the entry
 * @param value value of the entry
 */
inline int yfh_cursor_get(struct yfh_cursor * cur, const char ** key, void ** value) {
    if (cur->current == NULL)
        return 1;

    if (key)
        *key = cur->current->key;
    if (value)
        *value = cur->current->value;
    return 0;
}

/**
 * Tries to find next element. Returns 1 if reached the end, 2 on other error 0 otherwise.
 */
int yfh_cursor_next(struct yfh_cursor * cur);

/**
 * Used to repoint the cursor at next element after yfh_remove_at.
 * Return has the same meaning as yfh_cursor_next.
 */
inline int yfh_cursor_recalibrate(struct yfh_cursor * cur) {
    if (cur->current != NULL)
        return 0;
    return yfh_cursor_next(cur);
}

/**
 * Like yfh_get, returns 1 if the key was not found (and the cursor points to nothing and the bucket is unspecified).
 * If the key was found, the cursor points to its entry, and the value (if not null) is set to point to its value.
 * @param value place to hold the value, if found; can be null
 */
int yfh_cursor_find(struct yfh_cursor * cur, const char * key, void ** value);

#endif /* UTIL_HASHMAP_H */
