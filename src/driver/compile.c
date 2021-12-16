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

    struct yf_project_compilation_data data;
    int numf, i;

    numf = yf_find_project_files(&data);
    
    /* Parse the frontend for all */
    for (i = 0; i < numf; ++i) {
        yf_run_frontend(data.files[i], args);
    }

    /* TODO - semantic analysis, code gen */
    return 0;

}

static int yf_compile_files(struct yf_args * args) {
    
    struct yf_individual_compilation_data data;
    int i;

    for (i = 0; i < args->num_files; ++i) {
        data.files[i] = malloc(sizeof (struct yf_file_compilation_data));
        data.files[i]->file_name = args->files[i];
        /* TODO - more data */
    }
    data.num_files = args->num_files;

    /* Parse the frontend for all */
    for (i = 0; i < args->num_files; ++i) {
        yf_run_frontend(data.files[i], args);
    }

    /* TODO - semantic analysis, code gen */
    return 0;

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
