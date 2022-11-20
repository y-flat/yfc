#include "list.h"

#include <stdlib.h>
#include <string.h>

#include <util/allocator.h>

yf_result yf_list_init(struct yf_list * list) {
    list->first = yf_malloc(sizeof(struct yf_list_block));
    list->first->numfull = 0;
    list->first->next = NULL;
    list->last = list->first;
    return YF_OK;
}

yf_result yf_list_next(struct yf_list_cursor * cur) {

    ++cur->list_index;
    ++cur->block_index;
    if (cur->block_index >= cur->block->numfull) {
        if (cur->block->next == NULL) {
            return YF_REACHED_END;
        } else {
            cur->block = cur->block->next;
            cur->block_index = 0;
        }
    } else {
        if (cur->block_index > cur->block->numfull) {
            return YF_REACHED_END;
        }
    }

    return YF_OK;

}

yf_result yf_list_add(struct yf_list * list, void * element) {

    struct yf_list_block * block = list->last;
    yf_result res;

    if (block == NULL) {
        if ((res = yf_list_init(list)) != YF_OK)
            return res;
        block = list->last;
    }

    /* Get to the end */
    /*while (block->next != NULL) {
        block = block->next;
    }*/

    if (block->numfull == YF_LIST_BLOCK_SIZE) {
        block->next = yf_malloc(sizeof(struct yf_list_block));
        block->next->next = NULL;
        block = block->next;
        block->numfull = 0;
        list->last = block;
    }

    /* Add the element */
    block->data[block->numfull] = element;
    block->numfull++;

    return YF_OK;

}

yf_result yf_list_merge(struct yf_list * dst, struct yf_list * src) {
    if (dst == src)
        return YF_ERROR;

    if (src->first == NULL)
        return YF_OK;

    struct yf_list_block ** pblock;
    for (pblock = &dst->first; *pblock; pblock = &(*pblock)->next) {}
    *pblock = src->first;
    dst->last = src->last;
    src->first = src->last = NULL;
    return YF_OK;
}

void yf_list_destroy(struct yf_list * list, int free_elements) {

    struct yf_list_block * block = list->first;
    struct yf_list_block * last; /* See below */

    int i;

    /* Go and free all blocks. */
    while (block) {
        last = block;
        block = block->next;
        if (free_elements) {
            for (i = 0; i < last->numfull; i++) {
                yf_free(last->data[i]);
            }
        }
        yf_free(last);
    }

}

/* External inline function definitions */
extern inline yf_result yf_list_get(struct yf_list_cursor * cur, void ** elem);
extern inline size_t yf_list_get_count(struct yf_list *list);
