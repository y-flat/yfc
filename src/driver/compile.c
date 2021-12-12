#include "compile.h"

#include <api/compilation-data.h>
#include <driver/compile.h>
#include <parser/parser.h>

/* Forward decls for whole file */
static int yf_compile_project(struct yf_args *);
static int yf_compile_files(struct yf_args *);
static int yf_run_frontend(struct yf_file_compilation_data *);

/**
 * This is it. This is the actual compile function for a set of arguments. It
 * just defers compilation to one of two functions, depending on whether
 * --project is enabled or not.
 */
int yf_run_compiler(struct yf_args * args) {
    
    if (args->project) {
        return yf_compile_project(args);
    } else {
        return yf_compile_files(args);
    }

}

static int yf_compile_project(struct yf_args * args) {
    /* TODO */
    return 0;
}

static int yf_compile_files(struct yf_args * args) {
    /* TODO */
    return 0;
}

/**
 * Run the lexing and parsing on one file.
 */
static int yf_run_frontend(struct yf_file_compilation_data * file) {
    return yf_parse_file(file->file_name, file->parse_tree);
}
