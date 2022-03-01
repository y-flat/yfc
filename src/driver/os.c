#include "os.h"

#include <stdio.h>
#include <stdlib.h>

/** TODO: Choose implementation based on platform */

/** Unix implementation */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/**
 * macOS closefrom hack
 * closefrom doesn't exist on macOS, so here's a mock of it.
 */
#ifdef __APPLE__
#define closefrom(fd) do { \
    int i; \
    for (i = fd; i < getdtablesize(); i++) { \
        close(i); \
    } \
} while (0)
#endif /* __APPLE__ */

int proc_exec(const char * const argv[], const file_open_descriptor descs[], int flags) {
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("Warning: fork failed");
        return -1;
    } else if (child_pid == 0) {
        int* descriptors = NULL;
        size_t descriptors_sz = 0;
        int nullfd = -1, maxfd = 0, max_used_fd;
        for (file_open_descriptor const* descriptor = descs; descriptor->target_fd != -1; ++descriptor) {
            if (descriptor->target_fd > maxfd)
                maxfd = descriptor->target_fd;
        }
        max_used_fd = maxfd;
        for (file_open_descriptor const* descriptor = descs; descriptor->target_fd != -1; ++descriptor) {
            if (descriptor->target_fd >= (int)descriptors_sz) {
                descriptors = realloc(descriptors, (descriptor->target_fd + 1) * sizeof(int));
                for (; descriptors_sz < descriptor->target_fd + 1; ++descriptors_sz)
                    descriptors[descriptors_sz] = YF_OS_FILE_CLOSED;
            }
            int fd = descriptor->source_fd;
            if (fd == YF_OS_FILE_DEVNULL)
            {
                if (nullfd == -1)
                {
                    nullfd = open("/dev/null", O_RDWR);
                    if (nullfd != -1)
                    {
                        dup2(nullfd, ++maxfd);
                        nullfd = maxfd;
                    }
                }
                fd = nullfd;
            }
            descriptors[descriptor->target_fd] = fd;
        }
        // TODO: Logic for keeping track of fd clobbers is too complicated for now
        int fd_dst = 0;
        while (fd_dst < (int)descriptors_sz) {
            int fd_src = descriptors[fd_dst];
            if (fd_src == YF_OS_FILE_CLOSED)
                close(fd_dst);
            else
                dup2(fd_src, fd_dst);
            ++fd_dst;
        }
        closefrom(max_used_fd);
        if (flags & YF_OS_USE_PATH)
            execvp(argv[0], (char **)argv);
        else
            execv(argv[0], (char **)argv);
        perror("Process not executed");
        abort();
    }

    int status;
    if (waitpid(child_pid, &status, 0) == -1) {
        perror("Warning: wait failed");
        return -2;
    }
    return WEXITSTATUS(status);
}
