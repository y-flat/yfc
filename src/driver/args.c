/**
 * Actual argument parsing routines.
 */

#include "args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <driver/help.h>

/**
 * Internal - set an argument structure to have an error.
 */
static void yf_set_error(struct yf_args * args) {
    args->error = 1;
    args->wanted_output = YF_ERROR;
}

/**
 * @brief Internal - check if strings are equal.
 */
#define STREQ(a, b) (strcmp(a, b) == 0)

/**
 * Check if the wanted action is not already set.
 */
static void yf_check_action(struct yf_args * args, enum yf_info_output out) {
    if (args->wanted_output != YF_NONE) {
        yf_set_error(args);
    } else {
        args->wanted_output = out;
    }
}

/**
 * Internal - add a file. Return 1 if too many files.
 */
static int yf_add_file(struct yf_args * args, char * file) {
    /* No actions allowed. e.g. : no "yfc --version foo.yf" */
    if (args->wanted_output != YF_NONE) {
        yf_set_error(args);
        return 1;
    }
    /* No project allowed. */
    if (args->project) {
        yf_set_error(args);
        return 1;
    }
    yf_list_add(&args->files, file);
    return 0;
}

void yf_parse_args(int argc, char ** argv, struct yf_args * args) {
    
    int i;
    char * arg;
    
    /* If the next option we're parsing is the native C compiler name */
    bool want_compiler_name = false;
    bool want_compiler_type = false;

    /* Zero the args structure. */
    memset(args, 0, sizeof *args);
    args->wanted_output = YF_NONE;
    yf_list_init(&args->files);

    /* Start at 1 - avoid program name */
    for (i = 1; i < argc; ++i) {

        arg = argv[i];

        /* Skip all flag-checking if compiler name expected */
        if (want_compiler_name) {
            want_compiler_name = false;
            if (!args->compiler) {
                args->compiler = arg;
            } else {
                /* Name has already been set */
                yf_set_error(args);
                return;
            }
            continue;
        }

        if (want_compiler_type) {
            want_compiler_type = false;
            if (args->compiler_class == YF_COMPILER_UNKNOWN) {
                if (STREQ(arg, "gcc")) {
                    args->compiler_class = YF_COMPILER_GCC;
                }
                else if (STREQ(arg, "msvc")) {
                    args->compiler_class = YF_COMPILER_MSVC;
                }
                else {
                    /* Invalid compiler type */
                    yf_set_error(args);
                }
            } else {
                /* Name has already been set */
                yf_set_error(args);
                return;
            }
            continue;
        }

        if (arg[0] == '-') {
            ++arg;
            if (!arg[1]) {
                if (arg[0] == 'h') {
                    yf_check_action(args, YF_HELP);
                    continue;
                }

                if (arg[0] == 'v') {
                    yf_check_action(args, YF_VERSION);
                    continue;
                }
            }

            if (arg[0] == '-')
                ++arg;

            if (STREQ(arg, "help")) {
                yf_check_action(args, YF_HELP);
                continue;
            }

            if (STREQ(arg, "version")) {
                yf_check_action(args, YF_VERSION);
                continue;
            }

            if (STREQ(arg, "native-compiler")) {
                want_compiler_name = true;
                /* Make sure there actually is a compiler to parse after */
                if (i + 1 == argc) {
                    yf_set_error(args);
                    return;
                }
                continue;
            }

            if (STREQ(arg, "compiler-type")) {
                want_compiler_type = true;
                if (i + 1 == argc) {
                    yf_set_error(args);
                    return;
                }
                continue;
            }

            if (STREQ(arg, "project")) {
                args->project = 1;
                if (!yf_list_is_empty(&args->files)) {
                    /* --project specifies which files to use. */
                    yf_set_error(args);
                    return;
                }
                continue;
            }

            if (STREQ(arg, "dump-tokens")) {
                if (args->cstdump || args->just_semantics) {
                    yf_set_error(args);
                    return;
                }
                args->tdump = 1;
                continue;
            }

            if (STREQ(arg, "dump-cst")) {
                if (args->tdump || args->just_semantics) {
                    yf_set_error(args);
                    return;
                }
                args->cstdump = 1;
                continue;
            }

            if (STREQ(arg, "just-semantics")) {
                if (args->tdump || args->cstdump) {
                    yf_set_error(args);
                    return;
                }
                args->just_semantics = 1;
                continue;
            }

            if (STREQ(arg, "benchmark")) {
                if (args->profile || args->wanted_output != YF_NONE) {
                    yf_set_error(args);
                    return;
                }
                args->profile = 1;
                continue;
            }

            if (STREQ(arg, "dump-projfiles")) {
                args->dump_projfiles = 1;
                args->project = 1;
                continue;
            }

            if (STREQ(arg, "dump-commands")) {
                args->dump_commands = 1;
                continue;
            }

            if (STREQ(arg, "simulate-run")) {
                args->simulate_run = 1;
                args->dump_commands = 1;
                continue;
            }

            /* No other options are known. Yet. */
            yf_set_error(args);
            return;
        }

        /* TODO - file parsing, etc. */
        if (yf_add_file(args, arg)) {
            yf_set_error(args);
            return;
        }

    }

    if (args->wanted_output == YF_NONE && !args->project && yf_list_is_empty(&args->files)) {
        args->error = 1;
        args->wanted_output = YF_ERROR_NO_ARGS;
    }

}

bool yf_should_compile(struct yf_args * args) {
    return args->wanted_output == YF_NONE;
}

int yf_output_info(struct yf_args * args) {

    switch (args->wanted_output) {
        case YF_NONE:
            return 0;
        case YF_VERSION:
            printf("%s", VERSION_MSG);
            return 0;
        case YF_HELP:
            printf("%s", USAGE_MSG);
            return 0;
        case YF_ERROR:
            printf("%s", HELP_HINT_MSG);
            return 1;
        case YF_ERROR_NO_ARGS:
            printf("%s", NO_ARGS_MSG);
            return 1;
    }

}
