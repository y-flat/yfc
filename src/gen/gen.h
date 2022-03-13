/**
 * Interface for the C code generator.
 */

#ifndef GEN_GEN_H
#define GEN_GEN_H

#include <api/compilation-data.h>
#include <api/generation.h>

int yfg_gen(struct yf_compile_analyse_job * data, struct yf_gen_info * info);

#endif /* GEN_GEN_H */
