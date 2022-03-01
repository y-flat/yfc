#include "driver/args.h"
#define _GNU_SOURCE
#include <sys/mman.h>

#include "c-compiler.h"
#include "driver/os.h"

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

    int commf = memfd_create("compiler_path", 0);
    /* Hack to get around descriptor allocation quirks in proc_exec */
    dup2(commf, 50);
    commf = 50;

    const char * command[] = { "which", compiler, NULL };
    const file_open_descriptor descs[] = {
        { 0, YF_OS_FILE_DEVNULL },
        { 1, commf },
        { 2, YF_OS_FILE_DEVNULL },
        { -1, -1 }
    };

    /* "which" returns an error if the command doesn't exist */
    found = proc_exec(command, descs, YF_OS_USE_PATH) == 0;

    if (found && selected) {
        /* which produces extra endline */
        off_t sz = lseek(commf, -1, SEEK_END);

        lseek(commf, 0, SEEK_SET);
        char * buf = malloc(sz + 1);
        *selected = buf;
        read(commf, buf, sz);
        buf[sz] = 0;
    }

    close(commf);
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
