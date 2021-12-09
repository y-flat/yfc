/**
 * Command line argument structure.
 */

#ifndef DRIVER_ARGS_H
#define DRIVER_ARGS_H

struct yf_args {
    /* TODO */
};

/**
 * Stuff the args structure with all command line information, from the
 * provided arguments (passed through from main, presumably)
 */
void yf_parse_args(int argc, char ** argv, struct yf_args * args);

#endif /* DRIVER_ARGS_H */
