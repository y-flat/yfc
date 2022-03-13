/**
 * Structures for code generation.
 */

#ifndef API_GENERATION_H
#define API_GENERATION_H

struct yf_gen_info {

    char * yf_prefix; /* The prefix in Y-flat, like path.to.foo */
    char gen_prefix[256]; /* The prefix in generated code, like path$to$foo */
    int tab_depth; /* For the indentation level and proper formatting. */

};

#endif /* API_GENERATION_H */
