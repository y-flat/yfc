/** OS interface */

#ifndef DRIVER_OS_H
#define DRIVER_OS_H

#define YF_OS_FILE_CLOSED -1
#define YF_OS_FILE_DEVNULL -2

#define YF_OS_USE_PATH (1 << 0)

#include <stdint.h>

typedef struct {
    int target_fd;
    int source_fd; // can be OS_FILE_... constant
} file_open_descriptor;

typedef struct {
    intptr_t pid;
    int exit_code;
} process_handle;

/**
 * @param argv must be a NULL-terminated array of strings
 * @param descs must be an array of descriptors terminated with a descriptor whose target_fd is -1
 * @param flags YF_OS_USE_PATH: whether to use PATH to search for the program (if true), or to use the first argument as a file path directly (if false)
 * @return process exit code
 */
int proc_exec(const char * const argv[], const file_open_descriptor descs[], int flags);

/**
 * Same as above, except it allows to explicitly wait (for instance, to allow reading from/writing to a pipe)
 * @return 0 on success, otherwise nonzero
 */
int proc_open(process_handle * proc, const char * const argv[], const file_open_descriptor descs[], int flags);

/**
 * Waits for a process and saves the exit code
 * @return 0 on success, otherwise nonzero
 */
int proc_wait(process_handle * proc);

#endif /* DRIVER_OS_H */
