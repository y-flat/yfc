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
#include <util/hashmap.h>

struct yf_file_compilation_data {

    char * file_name; /* Where the source code is */
    char * sym_file; /* Where the symbols are stored */
    char * output_file; /* Where the C code is written */

    struct yf_parse_node parse_tree;

    struct yfs_symtab symtab;

    struct yfs_type_table types;

    struct yf_ast_node ast_tree;

    int error; /* If an error has occurred */

    /**
     * Are we parsing this file anew (recompiling it)? Two options:
     * 0:
     * - we read source code from file_name
     * - we dump symbol data to sym_file
     * 1:
     * - we read symbol data from sym_file because the symbol format is / will
     *   be the same as the source code format, and only use it to validate
     *   other files.
     */
    int parse_anew;

};

/**
 * TODO - distinguish project compilation and manual file compilation
 * ( --project vs. not )
 */
struct yf_project_compilation_data {

    /* To start - also for error messages */
    char * project_name;

    /* There are indeed a lot of pointers here, but this is so that a project
    with 5 files does not take up as much space as a project with 1000. */
    struct yf_hashmap * files;

    int num_files;

};

#endif /* API_COMPILATION_DATA_H */
