/**
 * Linked list implementation.
 */

#ifndef UTIL_LIST_H
#define UTIL_LIST_H

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

    /* Current view */
    struct yf_list_block * current;
    int current_index;

};

/**
 * The public API functions for lists - below
 */

/**
 * Initialize a new list. Return -1 if memory allocation has failed.
 */
int yf_list_init(struct yf_list * list);

/**
 * Get the currently pointed-to element. Returns -1 if we've passed the end, or
 * 0 otherwise.
 */
int yf_list_get(struct yf_list * list, void ** elem);

/**
 * Move to the next element. Return -1 if we've passed the end, or 0.
 */
int yf_list_next(struct yf_list * list);

/**
 * Reset to the beginning.
 */
void yf_list_reset(struct yf_list * list);

/**
 * Add an element. Returns -1 if we've run out of memory, or 0 otherwise.
 */
int yf_list_add(struct yf_list * list, void * element);

/**
 * Merges two lists together.
 * src will be empty after the operation, if successful
 */
int yf_list_merge(struct yf_list * dst, struct yf_list * src);

/**
 * Destroy a list.
 */
void yf_list_destroy(struct yf_list * list, int free_elements);

#endif /* UTIL_LIST_H */
