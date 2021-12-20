#include "list.h"

#include <stdlib.h>
#include <string.h>

#include <util/allocator.h>

int yf_list_init(struct yf_list * list) {
    list->current = NULL;
    list->current_index = YF_LIST_BLOCK_SIZE;
    return 0;
}

int yf_list_get(struct yf_list * list, void ** elem) {

    if (!list->current || (list->current_index > list->current->numfull)) {
        return -1;
    }

    *elem = list->current->data[list->current_index];
    return 0;

}

int yf_list_next(struct yf_list * list) {

    if (list->current_index == YF_LIST_BLOCK_SIZE - 1) {
        if (list->current->next == NULL) {
            return -1;
        } else {
            list->current = list->current->next;
            list->current_index = 0;
        }
    } else {
        ++list->current_index;
        if (!list->current || (list->current_index > list->current->numfull)) {
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

    struct yf_list_block * block = list->current;

    /* Get to the end of the list. */
    for (;;) {
        if (block == NULL || block->next == NULL) {
            break;
        }
        block = block->next;
    }

    if (!block || (block->numfull == YF_LIST_BLOCK_SIZE)) {
        if (!block) {
            block = yf_malloc(sizeof (struct yf_list_block));
        } else {
            block->next = yf_malloc(sizeof (struct yf_list_block));
            block = block->next;
        }
        if (!block) {
            return -1;
        }
        memset(block, 0, sizeof (struct yf_list_block));
        list->current_index = 0;
    }

    block->data[list->current_index] = element;
    ++block->numfull;

    return 0;

}

void yf_list_destroy(struct yf_list * list) {

    struct yf_list_block * block = list->current;
    struct yf_list_block * last; /* See below */

    if (!block) return;

    /* Go and free all blocks. */
    for (;;) {
        last = block;
        block = block->next;
        yf_free(last);
        if (!block) break;
    }

}
