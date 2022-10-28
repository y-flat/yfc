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
#include <util/list.h>
#include <util/hashmap.h>

enum yf_compilation_job_type {
    YF_COMPILATION_ANALYSE,
    YF_COMPILATION_COMPILE,
    YF_COMPILATION_EXEC,
};

enum yf_compilation_stage {
    YF_COMPILE_LEXONLY,
    YF_COMPILE_PARSEONLY,
    YF_COMPILE_ANALYSEONLY,
    YF_COMPILE_CODEGENONLY,
    YF_COMPILE_FULL,
};

struct yf_compilation_job {
    enum yf_compilation_job_type type;
};

/** Represents various info about a compilation unit */
struct yf_compilation_unit_info {

    char * file_name; /* Where the source code is, like src/path/to/foo.yf */
    char * file_prefix; /* Identifier prefix, like path.to.foo */
    char * sym_file; /* Where the symbols are stored */
    char * output_file; /* Where the C code is written */

    /**
     * Are we parsing this file anew (recompiling it)? Two options:
     * 0:
     * - we read symbol data from sym_file because the symbol format is / will
     *   be the same as the source code format, and only use it to validate
     *   other files.
     * 1:
     * - we read source code from file_name
     * - we dump symbol data to sym_file
     */
    int parse_anew;

};

/** Perform lexical & syntatic analysis on a source file, then build a symbol table */
struct yf_compile_analyse_job {
    struct yf_compilation_job job;

    enum yf_compilation_stage stage;

    struct yf_compilation_unit_info * unit_info;

    struct yf_parse_node parse_tree;

    struct yfs_symtab symtab;

    struct yfs_type_table types;

    struct yf_ast_node ast_tree;

};

/** Compile output file and a symbol file from a compilation unit */
struct yf_compile_compile_job {
    struct yf_compilation_job job;

    struct yf_compile_analyse_job * unit;

};

struct yf_compile_exec_job {
    struct yf_compilation_job job;

    /** Null-terminated array of arguments (argument are not owned, array is) */
    const char ** command;
};

/*
 * Stores all jobs that need to be done and all data that's required to compile
 */
struct yf_compilation_data {

    /**
     * The jobs required to complete the compilation
     * @item_type yf_compilation_job
     */
    struct yf_list jobs;

    /** Name of the project, if any (can be NULL) */
    char * project_name;

    /**
     * Will contain pointers to translation_units' symbol tables as they are analysed
     * @item_type yfs_symtab
     */
    struct yf_hashmap symtables;

    /**
     * Holds additional references that will be cleaned
     * @item_type ?
     */
    struct yf_list garbage;

};

#endif /* API_COMPILATION_DATA_H */
