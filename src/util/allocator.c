#include "allocator.h"

#include <util/yfc-out.h>

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
