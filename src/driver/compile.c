#include "compile.h"

#include <stdio.h> /* fopen, etc. */
#include <stdlib.h> /* malloc */

#include <api/compilation-data.h>
#include <api/lexer-input.h>
#include <driver/compile.h>
#include <parser/parser.h>
#include <util/yfc-out.h>

/* Forward decls for whole file */
static int yf_compile_project(struct yf_args *);
static int yf_compile_files(struct yf_args *);
static int yf_run_frontend(struct yf_file_compilation_data *, struct yf_args *);
static int yf_find_project_files(struct yf_project_compilation_data *);
static int dump_tokens(struct yf_lexer *);
static int yf_build_symtab(struct yf_file_compilation_data *);
static int yf_validate_ast(struct yf_file_compilation_data *, struct yf_args *);
static int yf_run_backend(struct yf_file_compilation_data *, struct yf_args *);

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

static int yf_run_compiler_on_data(
    struct yf_project_compilation_data * data,
    struct yf_args * args
) {

    int i;

    /* Parse the frontend for all and create symtabs */
    for (i = 0; i < data->num_files; ++i) {
        yf_run_frontend(data->files[i], args);
        yf_build_symtab(data->files[i]);
    }

    /* Now validate everything. */
    for (i = 0; i < data->num_files; ++i) {
        yf_validate_ast(data->files[i], args);
    }

    /* Finally, generate code. */
    for (i = 0; i < data->num_files; ++i) {
        yf_run_backend(data->files[i], args);
    }

    return 0;

}

static int yf_compile_project(struct yf_args * args) {

    struct yf_project_compilation_data data;
    int numf;

    numf = yf_find_project_files(&data);
    data.num_files = numf;
    
    return yf_run_compiler_on_data(&data, args);

}

static int yf_compile_files(struct yf_args * args) {
    
    struct yf_project_compilation_data data;
    int i;

    for (i = 0; i < args->num_files; ++i) {
        data.files[i] = malloc(sizeof (struct yf_file_compilation_data));
        data.files[i]->file_name = args->files[i];
        /* TODO - more data */
    }

    data.num_files = args->num_files;

    return yf_run_compiler_on_data(&data, args);

}

/**
 * Run the lexing and parsing on one file.
 */
static int yf_run_frontend(
    struct yf_file_compilation_data * file, struct yf_args * args
) {

    struct yf_lexer_input input;
    struct yf_lexer lexer;
    FILE * file_src;

    file_src = fopen(file->file_name, "r");
    if (!file_src) {
        YF_PRINT_ERROR("Could not open file %s", file->file_name);
        return 1;
    }
    
    input = (struct yf_lexer_input) {
        .input = file_src,
        .getc = (int (*)(void*)) getc,
        .ungetc = (int (*)(int, void*)) ungetc
    };

    yfl_init(&lexer, &input);

    if (args->tdump) {
        return dump_tokens(&lexer);
    } else {
        return yf_parse(&lexer, &file->parse_tree);
    }

}

/**
 * Stuff the project compilation data with all files that need to be compiled.
 * Returns the number of files.
 */
static int yf_find_project_files(struct yf_project_compilation_data * data) {
    /* TODO */
    return 0;
}

/**
 * Dump all file tokens.
 */
static int dump_tokens(struct yf_lexer * lexer) {

    struct yf_token token;
    for (;;) {
        yfl_lex(lexer, &token);
        if (token.type == YFT_EOF) {
            break;
        }
        printf(
            "%20s, line: %3d, col: %3d, type: %20s\n",
            token.data, token.lineno, token.colno, yf_get_toktype(token.type)
        );
    }

    return -1; /* Indicates that nothing should else should be done (meaning no
    semantic analysis, etc. */

}

/**
 * Build a table of all externally visible symbols.
 */
static int yf_build_symtab(struct yf_file_compilation_data * data) {

    /* TODO */
    return 0;

}

/**
 * Create an AST from the concrete syntax tree, and validate it against the
 * combined symbol tables loaded so far.
 */
static int yf_validate_ast(
    struct yf_file_compilation_data * data,
    struct yf_args * args
) {

    /* TODO */
    return 0;

}

/**
 * Generate C code, run the C compiler, and link the resulting binary.
 */
static int yf_run_backend(
    struct yf_file_compilation_data * data,
    struct yf_args * args
) {

    /* TODO */
    return 0;

}
