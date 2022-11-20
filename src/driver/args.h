/**
 * Command line argument structure.
 */

#ifndef DRIVER_ARGS_H
#define DRIVER_ARGS_H

#include <stdbool.h>

#include <util/list.h>

/**
 * Any of the possible outputs wanted.
 */
enum yf_info_output {
    YF_INFO_NONE,
    YF_INFO_VERSION,
    YF_INFO_HELP,
    YF_INFO_ERROR,
    YF_INFO_ERROR_NO_ARGS, /* SPECIFICALLY if no arguments are given. */
};

enum yf_compiler_class {
    YF_COMPILER_UNKNOWN,
    YF_COMPILER_GCC, // A gcc-like compiler
    YF_COMPILER_MSVC, // MS Visual C compiler (unsupported)
};

struct yf_args {
    
    /**
     * The wanted output message, or none if just compiling.
     */
    enum yf_info_output wanted_output;

    /**
     * Whether an error has occurred.
     */
    bool error;

    /**
     * Which compiler the user wants to use
     * This is a pointer rather than an array in case the user has some insanely
     * convoluted path or whatever. It still can't be > 255.
     * Set to NULL if none is specified.
     */
    const char * compiler;

    /**
     * Compiler chosen by the frontend
     */
    const char * selected_compiler;
    enum yf_compiler_class compiler_class;

    /**
     * The indiviidual files to compile.
     * @item_type char *
     */
    struct yf_list files;

    /**
     * Project flag, for if we're doing project setup and compiling all files in
     * it, instead of the user manually passing in files.
     */
    bool project;

    /**
     * Are we just dumping tokens for each file?
     */
    bool tdump;

    /**
     * How about ... just dumping the CST?
     */
    bool cstdump;

    /**
     * What about ... not trying to generate any code?
     */
    bool just_semantics;

    /**
     * Finally, do we compile the generated code?
     */
    bool run_c_comp;

    /**
     * Should we be profiling how long it takes?
     */
    bool profile;

    /**
     * Should we be dumping out all the files in a project?
     */
    bool dump_projfiles;

    /**
     * Should we be dumping compiler invocations?
     */
    bool dump_commands;

    /**
     * Should we only print the jobs?
     */
    bool simulate_run;

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
bool yf_should_compile(struct yf_args * args);

/**
 * Output any information requested, like a version number. This should ONLY be
 * called if yf_should_compile() returns 0. Also, if there's an error in the
 * arguments, this will output the diagnostic. This function exits the program
 * (with a 0 value if no error and 1 otherwise).
 */
int yf_output_info(struct yf_args * args);

#endif /* DRIVER_ARGS_H */
