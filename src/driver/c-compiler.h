/**
 * Determine the compiler used to compile the generated C code.
 */

#ifndef DRIVER_C_COMPILER_H
#define DRIVER_C_COMPILER_H

#include <driver/args.h>

/**
 * An access to the compiler used to compile the generated C code.
 */
extern char YF_C_COMPILER[256];

enum yf_c_compiler_status {
    YF_COMPILER_OK,
    YF_SPECIFIED_COMPILER_NOT_FOUND,
    YF_NO_COMPILER_FOUND,
    YF_COMPILER_NAME_OVERFLOW,
};

/**
 * Determine the compiler and fill the C_COMPILER variable with its value. USE
 * THIS BEFORE USING C_COMPILER.
 * It takes the args struct as input because the ussre can specify a C compiler
 * to use.
 * Returns:
 * YF_OK - if a compiler was found and all is well.
 * YF_SPECIFIED_COMPILER_NOT_FOUND - if the user specified a compiler but it
 * does not exist on this machine
 * YF_NO_COMPILER_FOUND - if no compiler was found on this machine
 */
enum yf_c_compiler_status yf_determine_c_compiler(struct yf_args *);

#endif /* DRIVER_C_COMPILER_H */
