#include "driver/args.h"

#include "c-compiler.h"
#include "driver/os.h"

#include <fcntl.h>
#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strlen, strcpy */
#include <unistd.h>

/**
 * Internal - determine whether a compiler exists on this machine.
 * This runs the command and sees if a non-existence error happens.
 */
static int compiler_exists(const char * compiler, const char ** selected) {

    bool found;

    int commfp[2];
    if (pipe(commfp))
        abort();

    /* Hack to get around descriptor allocation quirks in proc_exec */
    dup2(commfp[1], 50);
    close(commfp[1]);
    commfp[1] = 50;

    const char * command[] = { "which", compiler, NULL };
    const file_open_descriptor descs[] = {
        { 0, YF_OS_FILE_DEVNULL },
        { 1, commfp[1] },
        { 2, YF_OS_FILE_DEVNULL },
        { -1, -1 }
    };

    process_handle proc;
    if (proc_open(&proc, command, descs, YF_OS_USE_PATH)) {
        close(commfp[0]);
        close(commfp[1]);
        return false;
    }

    close(commfp[1]);

    size_t buf_size = 255;
    size_t buf_idx = 0;
    char * buf = malloc(255);

    ssize_t read_sz = 0;
    while ((read_sz = read(commfp[0], buf + buf_idx, buf_size - buf_idx)) > 0) {
        buf_idx += read_sz;
        if (buf_size - buf_idx < 10)
            buf = realloc(buf, buf_size *= 2);
    }

    if (read_sz < 0)
        abort();

    /* read_sz is 0 */
    if (proc_wait(&proc))
        abort();

    /* "which" returns an error if the command doesn't exist */
    found = proc.exit_code == 0;

    if (found && selected) {
        /* which produces extra endline; replace with NUL */
        buf[buf_idx - 1] = 0;
        *selected = buf;
    }

    close(commfp[0]);
    return found;

}

enum yf_c_compiler_status yf_determine_c_compiler(struct yf_args * args) {

    /* Check if the user has specified a compiler */
    if (args->compiler) {
        if (compiler_exists(args->compiler, &args->selected_compiler)) {
            if (args->compiler_class == YF_COMPILER_UNKNOWN)
                args->compiler_class = YF_COMPILER_GCC;
            return YF_COMPILER_OK;
        } else {
            return YF_SPECIFIED_COMPILER_NOT_FOUND;
        }
    }

    if (args->compiler_class != YF_COMPILER_UNKNOWN) {
        return YF_COMPILER_NO_CLASS;
    }

/* Set the compiler. I don't want to type this out for a million names. */
#define SET_COMPILER_IF_POSSIBLE(name, cls) do { \
    if (compiler_exists(name, &args->selected_compiler)) { \
        args->compiler_class = cls; \
        return YF_COMPILER_OK; \
    } \
} while (0)

    /* clang is better. I said it. */
    SET_COMPILER_IF_POSSIBLE("clang", YF_COMPILER_GCC);

    /* NOW try gcc. */
    SET_COMPILER_IF_POSSIBLE("gcc", YF_COMPILER_GCC);

    /* Now some others. */
    SET_COMPILER_IF_POSSIBLE("cc", YF_COMPILER_GCC);
    SET_COMPILER_IF_POSSIBLE("tcc", YF_COMPILER_GCC);

    /* TODO - add more? */

    /* None found. */
    return YF_NO_COMPILER_FOUND;

#undef SET_COMPILER_IF_POSSIBLE

}
