/**
 * Linked list implementation.
 */

#ifndef UTIL_LIST_H
#define UTIL_LIST_H

#include <stddef.h>
#include <stdbool.h>
#include "result.h"
#include "platform.h"

/**
 * How it works - each linked list is theoretically a list of nodes with a
 * pointer to the next one. But that's kind of slow in practice, and a waste of
 * pointers, so what we're going to do is to have a linked list of blocks
 * of memory, with some number of elements in each block.
 */

#define YF_LIST_BLOCK_SIZE 64

struct yf_list_block {
    struct yf_list_block * next;
    void * data[YF_LIST_BLOCK_SIZE];
    int numfull; /* The first empty block. */
};

/**
 * It's only a pointer to the first block and an indication of where we are.
 */
struct yf_list {

    struct yf_list_block * first;
    struct yf_list_block * last;

};

struct yf_list_cursor {

    struct yf_list * list;
    struct yf_list_block * block;
    int list_index, block_index;

};

/**
 * The public API functions for lists - below
 */

/**
 * Initialize a new list.
 */
yf_nodiscard yf_result yf_list_init(struct yf_list * list);

/**
 * Get the currently pointed-to element. Returns -1 if we've passed the end, or
 * 0 otherwise.
 */
yf_nodiscard inline yf_result yf_list_get(struct yf_list_cursor * cur, void ** elem) {

    if (cur->block_index >= cur->block->numfull) {
        return YF_REACHED_END;
    }

    *elem = cur->block->data[cur->block_index];
    return YF_OK;

}

/**
 * Move to the next element. Return -1 if we've passed the end, or 0.
 */
yf_nodiscard yf_result yf_list_next(struct yf_list_cursor * cur);

/**
 * Reset to the beginning.
 * If list is NULL, uses the last used list in cursor, it is undefined behaviour if the cursor hasn't been used before.
 */
yf_always_inline void yf_list_reset_cursor(struct yf_list_cursor * cur, struct yf_list * list) {

    if (list != NULL) {
        cur->list = list;
    }

    cur->block = cur->list->first;
    cur->block_index = 0;
    cur->list_index = 0;

}

/**
 * Add an element.
 */
yf_nodiscard yf_result yf_list_add(struct yf_list * list, void * element);

/**
 * Merges two lists together.
 * src will be empty after the operation, if successful.
 */
yf_nodiscard yf_result yf_list_merge(struct yf_list * dst, struct yf_list * src);

/**
 * Destroy a list.
 */
void yf_list_destroy(struct yf_list * list, int free_elements);

/**
 * Fast check if list is empty.
 */
yf_always_inline bool yf_list_is_empty(struct yf_list * list) {
    if (list->first == NULL)
        return true;

    return list->first->numfull == 0;
}

/**
 * Gets the total element count of the list
 */
inline size_t yf_list_get_count(struct yf_list * list) {
    struct yf_list_cursor cur;
    size_t cnt = 0;

    if (list->first == NULL || list->first->numfull == 0)
        return 0;

    yf_list_reset_cursor(&cur, list);
    do ++cnt; while (yf_list_next(&cur) == 0);
    return cnt;
}

/**
 * WARNING! This macro introduces a variable into outer scope and may thus be incompatible with constructs as if ... else ... without block scope.
 * @param list must be an lvalue that is safe to evaluate multiple times (like a variable)
 * @param out an lvalue denoting the element
 */
#define YF_LIST_FOREACH(list, out) \
    struct yf_list_cursor YF_LIST_CURSOR; \
    YF_LIST_FOREACH_CUR(YF_LIST_CURSOR, list, out)

/**
 * @param cur cursor variable/lvalue to use
 * @param list must be an lvalue that is safe to evaluate multiple times (like a variable)
 * @param out an lvalue denoting the element
 */
#define YF_LIST_FOREACH_CUR(cur, list, out) \
    for (yf_list_reset_cursor(&(cur), &(list)); yf_list_get(&(cur), (void **)&(out)) == YF_OK; (void)yf_list_next(&(cur)))

#endif /* UTIL_LIST_H */
