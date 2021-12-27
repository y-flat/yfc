/**
 * All data the compiler needs at once.
 * One might think it would be enough to have a parse tree and whatnot, but
 * all the files in a project need to get symbol data from each other and so
 * forth. This file is just for a project compilation structure, and an
 * individual file compilation.
 */

#ifndef API_COMPILATION_DATA_H
#define API_COMPILATION_DATA_H

#include <api/abstract-tree.h>
#include <api/concrete-tree.h>
#include <api/sym.h>

struct yf_file_compilation_data {

    /* To start - for error messages */
    const char * file_name;

    struct yf_parse_node parse_tree;

    struct yfs_symtab symtab;

    struct yf_ast_node ast_tree;

};

struct yf_project_compilation_data {

    /* To start - also for error messages */
    const char * project_name;

    /* There are indeed a lot of pointers here, but this is so that a project\
    with 5 files does not take up as much space as a project with 1000. */
    struct yf_file_compilation_data * files [1000];

    int num_files;

};

#endif /* API_COMPILATION_DATA_H */
