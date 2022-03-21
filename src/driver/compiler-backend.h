/**
 * An interface for the compiler backend.
 */

#ifndef DRIVER_COMPILER_BACKEND_H
#define DRIVER_COMPILER_BACKEND_H

#include <api/compilation-data.h>
#include <driver/args.h>

void yf_print_command(
    struct yf_compile_exec_job *
);

int yf_exec_command(
    struct yf_compile_exec_job *
);

int yf_backend_find_compiler(
    struct yf_args *
);

/** Returns the name of output object file */
char * yf_backend_add_compile_job(
    struct yf_compilation_data *,
    struct yf_args *,
    struct yf_compilation_unit_info *
);

int yf_backend_add_link_job(
    struct yf_compilation_data *,
    struct yf_args *,
    struct yf_list * object_list
);

int yf_backend_generate_code(
    struct yf_compile_analyse_job *
);

/**
 * Make sure that there is exactly one "main" function. Returns 0 on success.
 */
int yf_ensure_entry_point(
    struct yf_compilation_data *
);

#endif /* DRIVER_COMPILER_BACKEND_H */
