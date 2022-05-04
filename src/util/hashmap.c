#include "hashmap.h"

#include <string.h>

#include <util/allocator.h>

#define MAX_STEPS 10

/**
 * Hashmap cursor used internally by the hashmap functions.
 */
struct yfh_internal_cursor {
    struct yf_hashmap * hashmap;
    struct yfh_bucket ** bucket;
    struct yfh_bucket ** position;
};

static unsigned long hash(const char * key);

/**
 * Internal function to move the cursor to the position before the given key, if found.
 * Used by various hash map functions.
 * Returns true if the key has been found, false otherwise.
 * Note: this is a reverse of the conventions used by other functions of this kind.
 */
static bool yfh_cursor_find_before(struct yfh_internal_cursor * cur, const char * key) {

    unsigned long bucket = hash(key) % cur->hashmap->num_buckets;

    cur->bucket = cur->hashmap->buckets + bucket;
    for (cur->position = cur->bucket; *cur->position; cur->position = &(*cur->position)->next) {
        if (!strcmp(key, (*cur->position)->key))
            return true;
    }

    return false;

}

void yfh_destroy(struct yf_hashmap * hm, void (*cleanup)(void *)) {

    unsigned index, num_buckets = hm->num_buckets;

    for (index = 0; index < num_buckets; ++index) {
        struct yfh_bucket * bucket, * last;
        for (bucket = hm->buckets[index]; bucket; ) {
            last = bucket;
            bucket = bucket->next;
            if (cleanup)
                cleanup(last->value);
            free(last->key);
            free(last);
        }
    }

    free(hm->buckets);

}

/** TODO: Not implemented yet */
int yfh_rehash(struct yf_hashmap *, unsigned hint) {
    return 1;
}

int yfh_set(struct yf_hashmap * hm, const char * key, void * value) {

    struct yfh_internal_cursor cursor;
    struct yfh_bucket * bucket;
    cursor.hashmap = hm;

    if (yfh_cursor_find_before(&cursor, key)) {
        (*cursor.position)->value = value;
        return 0;
    }

    bucket = yf_malloc(sizeof(struct yfh_bucket));
    *cursor.position = bucket;
    bucket->key = yf_strdup(key);
    bucket->value = value;
    return 0;

}

int yfh_remove(struct yf_hashmap * hm, const char * key, void (*cleanup)(void *)) {

    struct yfh_internal_cursor cursor;
    struct yfh_bucket * bucket;
    cursor.hashmap = hm;

    if (!yfh_cursor_find_before(&cursor, key))
        return 1;

    bucket = *cursor.position;
    if (cleanup)
        cleanup(bucket->value);
    *cursor.position = bucket->next;
    free(bucket->key);
    free(bucket);
    return 0;

}

int yfh_remove_at(struct yfh_cursor * cur, void (*cleanup)(void *)) {

    /* We have to backtrack a little because we need to update the previouse pointer */
    struct yfh_bucket ** before_ptr;
    struct yfh_bucket * bucket;

    if (cur->bucket == NULL || cur->current == NULL)
        return 1;

    for (before_ptr = cur->bucket; *before_ptr; before_ptr = &(*before_ptr)->next) {
        if (*before_ptr == cur->current)
            break;
    }

    bucket = *before_ptr;

    if (cleanup)
        cleanup(bucket->value);
    *before_ptr = bucket->next;
    cur->current = *before_ptr;
    free(bucket->key);
    free(bucket);
    return 0;
}

int yfh_get(struct yf_hashmap * hm, const char * key, void ** value) {

    struct yfh_internal_cursor cursor;
    cursor.hashmap = hm;

    if (!yfh_cursor_find_before(&cursor, key))
        return 1;

    *value = (*cursor.position)->value;
    return 0;

}

int yfh_cursor_next(struct yfh_cursor * cur) {

    if (cur->hashmap == NULL)
        return 2;

    /* When a cursor is initialised, bucket is null */
    if (cur->bucket == NULL) {
        cur->bucket = cur->hashmap->buckets;
        cur->current = *cur->bucket;
        if (cur->current != NULL)
            return 0;
    }

    /* If we're pointing at an entry, move to the next one */
    if (cur->current != NULL) {
        cur->current = cur->current->next;
    }

    /* When a cursor points past a bucket list (or an empty bucket), advance to the next bucket */
    while (cur->current == NULL) {
        ++cur->bucket;
        /* Check if we reached an end */
        if (cur->bucket - cur->hashmap->buckets >= cur->hashmap->num_buckets) {
            cur->bucket = NULL;
            return 1;
        }

        cur->current = *cur->bucket;
    }

    return 0;

}

int yfh_cursor_find(struct yfh_cursor * cur, const char * key, void ** value) {

    /* We're gonna cheat a little bit */
    struct yfh_internal_cursor * internal_cur = (struct yfh_internal_cursor*)cur;

    if (cur->hashmap == NULL)
        return 1;

    if (!yfh_cursor_find_before(internal_cur, key)) {
        cur->current = NULL;
        return 1;
    }

    /* Fixup cursor pointer to point to the actual entry */
    cur->current = *internal_cur->position;
    if (value)
        *value = cur->current->value;
    return 0;

}

/* Credit - djb2 */
static unsigned long hash(const char * key) {

    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;

}
