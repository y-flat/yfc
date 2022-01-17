/**
 * An interface for the compiler backend.
 */

#ifndef DRIVER_COMPILER_BACKEND_H
#define DRIVER_COMPILER_BACKEND_H

#include <api/compilation-data.h>
#include <driver/args.h>

int yf_run_backend(
    struct yf_project_compilation_data *, struct yf_args *
);

#endif /* DRIVER_COMPILER_BACKEND_H */
