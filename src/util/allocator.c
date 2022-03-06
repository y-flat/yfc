#include "allocator.h"

#include <util/yfc-out.h>
#include <string.h>

void * yf_malloc(size_t size) {

    void * ret;
    ret = malloc(size);

    if (ret == NULL) {
        YF_PRINT_ERROR("Allocation of %zu bytes failed", size);
    }

    /* Return anyway, the error will be printed out before. */
    return ret;

}

void yf_free(void * ptr) {
    free(ptr);
}

/**
 * A version of strcpy that returns pointer to the terminating NUL-byte for faster concatenations
 */
char * yf_strcpy(char * restrict dst, const char * src) {
    size_t sz = strlen(src);
    memcpy(dst, src, sz + 1);
    return dst + sz;
}

char * yf_strdup(const char * src) {
    char * dst;
    size_t sz = strlen(src);
    dst = yf_malloc(sz + 1);
    memcpy(dst, src, sz + 1);
    return dst;
}
