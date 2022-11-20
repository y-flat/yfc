#include "hashmap.h"

#include <string.h>

#include <util/allocator.h>

#define MAX_STEPS 10

static unsigned long hash(const char * key);

/**
 * Internal function to move the cursor to the position before the given key, if found.
 * Used by various hash map functions.
 * Returns true if the key has been found, false otherwise.
 * Note: this is a reverse of the conventions used by other functions of this kind.
 */
static bool yfh_cursor_find_before(struct yfh_cursor * cur, const char * key) {

    unsigned long bucket = hash(key) % cur->hashmap->num_buckets;

    cur->bucket = cur->hashmap->buckets + bucket;
    for (cur->current = cur->bucket; *cur->current; cur->current = &(*cur->current)->next) {
        if (!strcmp(key, (*cur->current)->key))
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
yf_result yfh_rehash(struct yf_hashmap * hm, unsigned hint) {
    return YF_ERROR;
}

yf_result yfh_set(struct yf_hashmap * hm, const char * key, void * value) {

    struct yfh_cursor cursor;
    struct yfh_bucket * bucket;
    cursor.hashmap = hm;

    if (yfh_cursor_find_before(&cursor, key)) {
        (*cursor.current)->value = value;
        return YF_OK;
    }

    bucket = yf_malloc(sizeof(struct yfh_bucket));
    *cursor.current = bucket;
    bucket->key = yf_strdup(key);
    bucket->value = value;
    bucket->next = NULL;
    return YF_OK;

}

yf_result yfh_remove(struct yf_hashmap * hm, const char * key, void (*cleanup)(void *)) {

    struct yfh_cursor cursor;
    struct yfh_bucket * bucket;
    cursor.hashmap = hm;

    if (!yfh_cursor_find_before(&cursor, key))
        return YF_ERROR;

    bucket = *cursor.current;
    if (cleanup)
        cleanup(bucket->value);
    *cursor.current = bucket->next;
    free(bucket->key);
    free(bucket);
    return YF_OK;

}

yf_result yfh_remove_at(struct yfh_cursor * cur, void (*cleanup)(void *)) {

    /* We have to backtrack a little because we need to update the previous pointer */
    struct yfh_bucket * bucket;

    if (cur->bucket == NULL || cur->current == NULL || *cur->current == NULL)
        return YF_ERROR;

    bucket = *cur->current;

    if (cleanup)
        cleanup(bucket->value);
    *cur->current = bucket->next;
    free(bucket->key);
    free(bucket);
    return YF_OK;
}

yf_result yfh_get(struct yf_hashmap * hm, const char * key, void ** value) {

    struct yfh_cursor cursor;
    cursor.hashmap = hm;

    if (!yfh_cursor_find_before(&cursor, key))
        return YF_ERROR;

    *value = (*cursor.current)->value;
    return YF_OK;

}

yf_result yfh_cursor_next(struct yfh_cursor * cur) {

    if (cur->hashmap == NULL)
        return YF_ERROR;

    /* When a cursor is initialised, bucket is null */
    if (cur->bucket == NULL) {
        cur->bucket = cur->hashmap->buckets;
        cur->current = cur->bucket;
        if (*cur->current != NULL)
            return YF_OK;
    }

    /* If we're pointing at an entry, move to the next one */
    if (*cur->current != NULL) {
        cur->current = &(**cur->current).next;
    }

    /* When a cursor points past a bucket list (or an empty bucket), advance to the next bucket */
    while (*cur->current == NULL) {
        ++cur->bucket;
        /* Check if we reached an end */
        if (cur->bucket - cur->hashmap->buckets >= cur->hashmap->num_buckets) {
            cur->bucket = NULL;
            return YF_REACHED_END;
        }

        cur->current = cur->bucket;
    }

    return YF_OK;

}

yf_result yfh_cursor_find(struct yfh_cursor * cur, const char * key, void ** value) {

    if (cur->hashmap == NULL)
        return YF_ERROR;

    if (!yfh_cursor_find_before(cur, key)) {
        cur->current = NULL;
        return YF_ERROR;
    }

    if (value)
        *value = (**cur->current).value;
    return YF_ERROR;

}

/* Credit - djb2 */
static unsigned long hash(const char * key) {

    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;

}
