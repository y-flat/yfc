#include "hashmap.h"

#include <string.h>

#include <util/allocator.h>

#define MAX_STEPS 10

struct yf_hashmap * yfh_new() {
    struct yf_hashmap * hm;
    hm = yf_malloc(sizeof (struct yf_hashmap));
    memset(hm, 0, sizeof (struct yf_hashmap));
    hm->buckets = yf_malloc(sizeof (struct yfh_bucket) * YFH_BUCKETS);
    memset(hm->buckets, 0, sizeof (struct yfh_bucket) * YFH_BUCKETS);
    return hm;
}

void yfh_destroy(struct yf_hashmap * hm, int (*cleanup)(void *)) {

    int index;

    if (cleanup) {
        for (index = 0; index < YFH_BUCKETS; ++index) {
            if (hm->buckets[index].value)
                cleanup(hm->buckets[index].value);
        }
    }

    free(hm->buckets);
    free(hm);

}

static unsigned long hash(const char * key);

int yfh_set(struct yf_hashmap * hm, const char * key, void * value) {

    unsigned loc, newloc;
    int i;

    loc = hash(key) % YFH_BUCKETS;

    const char * pkey;

    /* Traverse forward until we find it. */
    for (i = 0; i < 10; ++loc, ++i) {
        newloc = (loc + i) % YFH_BUCKETS;
        pkey = hm->buckets[newloc].key;
        if (pkey && !strcmp(key, pkey)) {
            hm->buckets[newloc].value = value;
            return 0;
        }
        if (hm->buckets[newloc].key == NULL) {
            hm->buckets[newloc].value = value;
            hm->buckets[newloc].key   = key;
            return 0;
        }
    }

    return 1;

}

void * yfh_get(struct yf_hashmap * hm, char * key) {
    
    unsigned loc, newloc;
    int i;

    loc = hash(key) % YFH_BUCKETS;

    const char * pkey;

    /* Traverse forward until we find it. */
    for (i = 0; i < 10; ++loc, ++i) {
        newloc = (loc + i) % YFH_BUCKETS;
        pkey = hm->buckets[newloc].key;
        if (pkey && !strcmp(key, pkey)) {
            return hm->buckets[newloc].value;
        }
    }

    return NULL;

}

/* Credit - djb2 */
static unsigned long hash(const char * key) {

    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;

}
