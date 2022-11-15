/**
 * For projects -- dumping all symbols from other files as forward declarations
 * in the current file.
 */

#ifndef GEN_SYMGEN_H
#define GEN_SYMGEN_H

#include <stdio.h> /* FILE */

#include <api/compilation-data.h>

int yfg_output_syms(
    struct yf_compilation_data * cdata,
    struct yf_compile_analyse_job * data,
    FILE * out
);

#endif /* GEN_SYMGEN_H */
