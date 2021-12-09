/**
 * This file does one thing - takes in command-line arguments, and performs a
 * complete compilation process using the "utility routines" (actually more like
 * compiler components) found in the folders other than driver.
 */

#ifndef DRIVER_COMPILE_H
#define DRIVER_COMPILE_H

#include <driver/args.h>

/**
 * Do actual compilation with the given arguments. Return an error code through
 * the return value (non-zero) if something goes wrong.
 */
int yf_compile(struct yf_args * args);

#endif /* DRIVER_COMPILE_H */
