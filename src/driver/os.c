#include "os.h"

#include <util/platform.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int proc_exec(const char * const argv[], const file_open_descriptor descs[], int flags) {
    process_handle proc;
    int ret = proc_open(&proc, argv, descs, flags);
    if (ret != 0)
        return ret;
    ret = proc_wait(&proc);
    if (ret != 0)
        return ret;
    return proc.exit_code;
}

#if defined(YF_PLATFORM_UNIX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int proc_open(process_handle * proc, const char * const argv[], const file_open_descriptor descs[], int flags) {
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("Warning: fork failed");
        return -1;
    } else if (child_pid == 0) {
        int* descriptors = NULL;
        size_t descriptors_sz = 0;
        int nullfd = -1, maxfd = 0, max_used_fd;
        for (const file_open_descriptor * descriptor = descs; descriptor->target_fd != -1; ++descriptor) {
            if (descriptor->target_fd > maxfd)
                maxfd = descriptor->target_fd;
        }
        max_used_fd = maxfd;
        for (const file_open_descriptor * descriptor = descs; descriptor->target_fd != -1; ++descriptor) {
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
        if (flags & YF_OS_USE_PATH)
            execvp(argv[0], (char **)argv);
        else
            execv(argv[0], (char **)argv);
        perror("Process not executed");
        abort();
    }

    memset(proc, 0, sizeof *proc);
    proc->pid = child_pid;
    return 0;
}

int proc_wait(process_handle * proc) {
    int status;
    if (waitpid(proc->pid, &status, 0) == -1) {
        perror("Warning: wait failed");
        return -2;
    }
    proc->exit_code = WEXITSTATUS(status);
    return 0;
}
#elif defined(YF_PLATFORM_WINNT)
#include <Windows.h>

// Adapted from https://stackoverflow.com/questions/2611044/process-start-pass-html-code-to-exe-as-argument/2611075#2611075
static void EscapeBackslashes(char ** sb, char const* s, char const* begin)
{
    // Backslashes must be escaped if and only if they precede a double quote.
    while (*s == '\\')
    {
        *(*sb)++ = '\\';
        --s;

        if (s == begin)
            break;
    }
}

static const size_t cmdline_buffer_size = 256;
static void ArgvToCommandLine(char * buffer, const char * const args[])
{
    const char * buffer_end = buffer + cmdline_buffer_size;
    for (const char * s = *args; s; ++args)
    {
        const char * const sbeg = s;
        size_t s_len = strlen(s);
        const char * const send = s + s_len;
        *buffer++ = '"';
        // Escape double quotes (") and backslashes (\).
        while (1)
        {
            // Put this test first to support zero length strings.
            if (s >= send)
                break;

            const char * quote = strchr(s, '"');
            if (quote == NULL)
                break;

            for (const char * p = s; p != quote; ++p)
                *buffer++ = *p;
            EscapeBackslashes(&buffer, quote - 1, s);
            *buffer++ = '\\';
            *buffer++ = '"';
            s = quote + 1;
        }
        for (const char * p = s; p != send; ++p)
            *buffer++ = *p;
        EscapeBackslashes(&buffer, send - 1, sbeg);
        *buffer++ = '"';
        *buffer++ = ' ';
    }
    *buffer++ = 0;
    if (buffer > buffer_end) {
        /* Uh oh */
        abort();
    }
}

int proc_open(process_handle * proc, const char * const argv[], const file_open_descriptor descs[], int flags)
{
    HANDLE handles[3] = {0};
    for (const file_open_descriptor * descriptor = descs; descriptor->target_fd != -1; ++descriptor)
    {
        if (descriptor->target_fd > 2)
        {
            fputs("Warning: Windows process exec target fd > 2\n", stderr);
            return -1;
        }

        HANDLE handle = NULL;
        switch (descriptor->source_fd)
        {
            case YF_OS_FILE_CLOSED:
            case YF_OS_FILE_DEVNULL:
                break;

            case 0:
                handle = GetStdHandle(STD_INPUT_HANDLE);
                break;
            case 1:
                handle = GetStdHandle(STD_OUTPUT_HANDLE);
                break;
            case 2:
                handle = GetStdHandle(STD_ERROR_HANDLE);
                break;
            default:
                fputs("Warning: Windows process exec source fd > 2\n", stderr);
                return -1;
        }
        handles[descriptor->target_fd] = handle;
    }
    char cmd_line[cmdline_buffer_size];
    ArgvToCommandLine(cmd_line, argv);
    STARTUPINFOA startup_info{};
    startup_info.cb = sizeof startup_info;
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = handles[0];
    startup_info.hStdOutput = handles[1];
    startup_info.hStdError = handles[2];

    PROCESS_INFORMATION proc_info{};

    if (!CreateProcessA(argv[0], cmd_line, NULL, NULL, false, 0, NULL, NULL, &start_info, &proc_info))
    {
        return -1;
    }

    CloseHandle(proc_info.hThread);

    memset(proc, 0, sizeof *proc);
    proc->pid = proc_info.hProcess;
    return 0;
}

int proc_wait(process_handle * proc) {
    // Wait until child process exits.
    WaitForSingleObject(proc->pid, INFINITE);

    proc->exit_code = -2;
    GetExitCodeProcess(proc->pid, &proc->exit_code);

    CloseHandle(proc->pid);

    return 0;
}
#else /* YF_PLATFORM_UNIX | YF_PLATFORM_WINNT */
#error Unknown platform
#endif
