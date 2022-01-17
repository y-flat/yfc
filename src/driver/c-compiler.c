#include "c-compiler.h"

#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strlen, strcpy */

char YF_C_COMPILER[256];

/**
 * Internal - determine whether a compiler exists on this machine.
 * This runs the command and sees if a non-existence error happens.
 */
int compiler_exists(const char * compiler) {
    
    /* The command to run with system */
    char * command;
    /* The return value */
    int ret;
    
    /* Allocate buffer for command */
    command = malloc(
        strlen(compiler) +
        23 + /* The chars in "which [] 2>/dev/null 1>&2" (not counting []) */
        1 /* null terminator */
    );

    sprintf(command, "which %s 2>/dev/null 1>&2", compiler);

    /* "which" returns an error if the command doesn't exist */
    ret = system(command) == 0;

    free(command);
    return ret;

}

enum yf_c_compiler_status yf_determine_c_compiler(struct yf_args * args) {
    
    /* Check if the user has specified a compiler */
    if (args->compiler) {
        if (compiler_exists(args->compiler)) {
            /* Needs to be a short enough name! */
            if (strlen(args->compiler) > 255) {
                return YF_COMPILER_NAME_OVERFLOW;
            } else {
                strcpy(YF_C_COMPILER, args->compiler);
                return YF_COMPILER_OK;
            }
        } else {
            return YF_SPECIFIED_COMPILER_NOT_FOUND;
        }
    }

/* Set the compiler. I don't want to type this out for a million names. */
#define SET_COMPILER_IF_POSSIBLE(name) do { \
    if (compiler_exists(name)) { \
        strcpy(YF_C_COMPILER, name); \
        return YF_COMPILER_OK; \
    } \
} while (0)

    /* clang is better. I said it. */
    SET_COMPILER_IF_POSSIBLE("clang");

    /* NOW try gcc. */
    SET_COMPILER_IF_POSSIBLE("gcc");

    /* Now some others. */
    SET_COMPILER_IF_POSSIBLE("cc");
    SET_COMPILER_IF_POSSIBLE("tcc");

    /* TODO - add more? */

    /* None found. */
    return YF_NO_COMPILER_FOUND;

#undef SET_COMPILER_IF_POSSIBLE

}
