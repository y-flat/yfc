#include "list.h"

#include <stdlib.h>
#include <string.h>

#include <util/allocator.h>

int yf_list_init(struct yf_list * list) {
    list->first = yf_malloc(sizeof(struct yf_list_block));
    list->first->numfull = 0;
    list->first->next = NULL;
    list->current = list->first;
    list->current_index = 0;
    return 0;
}

int yf_list_get(struct yf_list * list, void ** elem) {

    if (list->current_index >= list->current->numfull) {
        return -1;
    }

    *elem = list->current->data[list->current_index];
    return 0;

}

int yf_list_next(struct yf_list * list) {

    ++list->current_index;
    if (list->current_index >= list->current->numfull) {
        if (list->current->next == NULL) {
            return -1;
        } else {
            list->current = list->current->next;
            list->current_index = 0;
        }
    } else {
        if (list->current_index > list->current->numfull) {
            return -1;
        }
    }

    return 0;

}

void yf_list_reset(struct yf_list * list) {

    list->current = list->first;
    list->current_index = 0;

}

int yf_list_add(struct yf_list * list, void * element) {

    struct yf_list_block * block = list->first;

    if (block == NULL)
        yf_list_init(list);

    /* Get to the end */
    while (block->next != NULL) {
        block = block->next;
    }

    if (block->numfull == YF_LIST_BLOCK_SIZE) {
        block->next = yf_malloc(sizeof(struct yf_list_block));
        block->next->next = NULL;
        block = block->next;
        block->numfull = 0;
    }

    /* Add the element */
    block->data[block->numfull] = element;
    block->numfull++;

    return 0;

}

int yf_list_merge(struct yf_list * dst, struct yf_list * src) {
    if (dst == src)
        return -1;

    if (src->first == NULL)
        return 0;

    struct yf_list_block ** pblock;
    for (pblock = &dst->first; *pblock; pblock = &(*pblock)->next) {}
    *pblock = src->first;
    src->first = src->current = NULL;
    src->current_index = 0;
    return 0;
}

void yf_list_destroy(struct yf_list * list, int free_elements) {

    struct yf_list_block * block = list->first;
    struct yf_list_block * last; /* See below */

    int i;

    if (!block) return;

    /* Go and free all blocks. */
    for (;;) {
        last = block;
        block = block->next;
        if (free_elements) {
            for (i = 0; i < last->numfull; i++) {
                yf_free(last->data[i]);
            }
        }
        yf_free(last);
        if (!block) break;
    }

}
