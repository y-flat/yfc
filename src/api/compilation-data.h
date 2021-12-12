/**
 * All data the compiler needs at once.
 * One might think it would be enough to have a parse tree and whatnot, but
 * all the files in a project need to get symbol data from each other and so
 * forth. This file is just for a project compilation structure, and an
 * individual file compilation.
 */

#ifndef API_COMPILATION_DATA_H
#define API_COMPILATION_DATA_H

struct yf_file_compilation_data {

    /* To start - for error messages */
    const char * file_name;

    /* TODO - actual data, like parse trees, etc. */

};

struct yf_project_compilation_data {

    /* To start - also for error messages */
    const char * project_name;

    /* There are indeed a lot of pointers here, but this is so that a project\
    with 5 files does not take up as much space ass a project with 1000. */
    struct yf_file_compilation_data * files [1000];

};

/**
 * To clarify - this name might be a bit misleading. This is NOT for compiling
 * one file, this is for compiling multiple files at once that are NOT part of a
 * project (for example, "yfc foo.yf bar.yf").
 */
struct yf_individual_compilation_data {

    struct yf_file_compilation_data * files[16];
    int num_files;

};

#endif /* API_COMPILATION_DATA_H */
