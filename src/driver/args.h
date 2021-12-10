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

/**
 * Whether or not compilation should even happen. I know one might think it
 * always should, but if the user does, say, yfc --help or --version, then no
 * compilation should happen. This ALSO returns 0 if there's some error in the
 * arguments given.
 */
int yf_should_compile(struct yf_args * args);

/**
 * Output any information requested, like a version number. This should ONLY be
 * called if yf_should_compile() returns 0. Also, if there's an error in the
 * arguments, this will output the diagnostic. This function exits the program
 * (with a 0 value if no error and 1 otherwise).
 */
void yf_output_info(struct yf_args * args);

#endif /* DRIVER_ARGS_H */
