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

void yf_parse_args(int argc, char ** argv, struct yf_args * args) {
    
    int i;
    char * flag;
    
    /* If the next option we're parsing is the native C compiler name */
    bool want_compiler_name = false;

    /* Zero the args structure. */
    memset(args, 0, sizeof *args);
    args->wanted_output = YF_NONE;

    if (argc == 1) {
        yf_check_action(args, YF_ERROR_NO_ARGS);
        return;
    }

    /* Start at 1 - avoid program name */
    for (i = 1; i < argc; ++i) {

        flag = argv[i];

        /* Skip all flag-checking if compiler name expected */
        if (want_compiler_name) {
            want_compiler_name = false;
            if (!args->compiler) {
                args->compiler = flag;
            } else {
                /* Name has already been set */
                yf_set_error(args);
                return;
            }
            continue;
        }

        if (STREQ(flag, "-h") || STREQ(flag, "--help")) {
            yf_check_action(args, YF_HELP);
            continue;
        }

        if (STREQ(flag, "-v") || STREQ(flag, "--version")) {
            yf_check_action(args, YF_VERSION);
            continue;
        }

        if (STREQ(flag, "-native-compiler")) {
            want_compiler_name = true;
            /* Make sure there actually is a compiler to parse after */
            if (i + 1 == argc) {
                yf_set_error(args);
                return;
            }
            continue;
        }

        /* TODO - file parsing, etc. */
        /* Other options are errors for now. */
        yf_set_error(args);

    }

}

bool yf_should_compile(struct yf_args * args) {
    return args->wanted_output == YF_NONE;
}

void yf_output_info(struct yf_args * args) {

    switch (args->wanted_output) {
        case YF_NONE:
            break;
        case YF_VERSION:
            printf("%s", VERSION_MSG);
            break;
        case YF_HELP:
            printf("%s", FLAGS_MSG);
            break;
        case YF_ERROR:
            printf("%s", HELP_HINT_MSG);
            break;
        case YF_ERROR_NO_ARGS:
            printf("%s", NO_ARGS_MSG);
            break;
    }

}
