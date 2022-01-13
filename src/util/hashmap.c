#include "hashmap.h"

#include <string.h>

#include <util/allocator.h>
#include <util/yfc-out.h>

/**
 * Perhaps larger in the future
 */
#define BUCKETS 100000

#define MAX_STEPS 10

/**
 * A bunch of buckets with values. Buckets also store the key so that collisions
 * can be avoided, by simply moving the value to be inserted into the next free
 * bucket.
 */
struct yf_hashmap {

    struct yfh_bucket {
        char * key;
        void * value;
    } * buckets;

};

struct yf_hashmap * yfh_new() {
    struct yf_hashmap * hm;
    hm = yf_malloc(sizeof (struct yf_hashmap));
    memset(hm, 0, sizeof (struct yf_hashmap));
    hm->buckets = yf_malloc(sizeof (struct yfh_bucket) * BUCKETS);
    memset(hm->buckets, 0, sizeof (struct yfh_bucket) * BUCKETS);
    return hm;
}

void yfh_destroy(struct yf_hashmap * hm, bool sub) {

    int index;

    if (sub) {
        for (index = 0; index < BUCKETS; ++index) {
            if (hm->buckets[index].value)
                free(hm->buckets[index].value);
        }
    }

    free(hm);

}

static int hash(char * key);

int yfh_set(struct yf_hashmap * hm, char * key, void * value) {

    int loc;
    int i;

    loc = hash(key);

    char * pkey;

    /* Traverse forward until we find it. */
    for (i = 0; i < 10; ++loc, ++i) {
        pkey = hm->buckets[loc + i].key;
        if (pkey && !strcmp(key, pkey)) {
            hm->buckets[loc + i].value = value;
            return 0;
        }
        if (hm->buckets[loc + i].key == NULL) {
            hm->buckets[loc + i].value = value;
            hm->buckets[loc + i].key   = key;
            return 0;
        }
    }

    return 1;

}

void * yfh_get(struct yf_hashmap * hm, char * key) {
    
    int loc;
    int i;

    loc = hash(key);

    char * pkey;

    /* Traverse forward until we find it. */
    for (i = 0; i < 10; ++loc, ++i) {
        pkey = hm->buckets[loc + i].key;
        if (pkey && !strcmp(key, pkey)) {
            return hm->buckets[loc + i].value;
        }
    }

    return NULL;

}

/* Credit - djb2 */
static int hash(char * key) {

    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;

}
