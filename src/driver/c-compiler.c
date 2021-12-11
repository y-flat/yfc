#include "c-compiler.h"

#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strlen */

const char YF_C_COMPILER[256];

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
    ret = system(command) != 0;

    free(command);
    return ret;

}

enum yf_c_compiler_status yf_determine_c_compiler(struct yf_args * args) {
    return YF_OK;
}
