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
    char output_file[256];

    struct yf_parse_node parse_tree;

    struct yfs_symtab symtab;

    struct yfs_type_table types;

    struct yf_ast_node ast_tree;

    int error; /* If an error has occurred */

};

struct yf_project_compilation_data {

    /* To start - also for error messages */
    char * project_name;

    /* There are indeed a lot of pointers here, but this is so that a project\
    with 5 files does not take up as much space as a project with 1000. */
    struct yf_file_compilation_data * files [1000];

    int num_files;

    /**
     * A map of module prefix -> symbol table.
     * Strictly speaking, this is not *just* external modules - local files that
     * don't need to be recompiled are still loaded in here, because they might
     * need to be imported.
     */
    struct yf_hashmap * ext_modules;

};

#endif /* API_COMPILATION_DATA_H */
