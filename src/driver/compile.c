#include "compile.h"

#include <stdio.h> /* fopen, etc. */
#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */
#include <sys/time.h> /* struct timeval */
#include <unistd.h> /* getcwd */

#include <api/compilation-data.h>
#include <api/cst-dump.h>
#include <api/lexer-input.h>
#include <driver/compiler-backend.h>
#include <driver/find-files.h>
#include <parser/parser.h>
#include <semantics/symtab.h>
#include <semantics/validate/validate.h>
#include <util/allocator.h>
#include <util/yfc-out.h>

/* Forward decls for whole file */
static int yf_compile_project(struct yf_args *);
static int yf_compile_files(struct yf_args *);
static int yf_run_frontend(struct yf_file_compilation_data *, struct yf_args *);
static int yf_find_project_files(struct yf_project_compilation_data *);
static int dump_tokens(struct yf_lexer *);
static int yf_build_symtab(struct yf_file_compilation_data *);
static int yf_validate_ast(
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata,
    struct yf_args * args
);
static int yf_do_cst_dump(struct yf_parse_node * tree);
static int yf_cleanup(struct yf_project_compilation_data *);

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

/* Local defines for profiling */
#define BEGIN_PROFILE() \
    gettimeofday(&step_begin, NULL)

#define END_PROFILE(step_name) do { \
    gettimeofday(&step_end, NULL); \
    if (args->profile) { \
        time_for_step = (step_end.tv_sec - step_begin.tv_sec) * 1000000 + \
            (step_end.tv_usec - step_begin.tv_usec); \
        YF_PRINT_DEFAULT( \
            "Time for %s: %f seconds", \
            step_name, \
            ( (double) time_for_step ) / 1000000.0 \
        ); \
    } \
} while (0)

    int i, err = 0;

    /* For profiling purposes only */
    double time_for_step, time_total;
    struct timeval total_begin, step_begin, step_end, total_end;

    /* Parse the frontend for all and create symtabs */
    BEGIN_PROFILE();
    total_begin = step_begin;
    for (i = 0; i < data->num_files; ++i) {
        data->files[i]->error = 0; /* Starting off clean */
        if (yf_run_frontend(data->files[i], args)) {
            data->files[i]->error = 1;
        }
        if (args->cstdump || args->tdump) {
            return data->files[i]->error;
        }
    }
    END_PROFILE("parsing");

    BEGIN_PROFILE();
    for (i = 0; i < data->num_files; ++i) {
        if (!data->files[i]->error) {
            if (yf_build_symtab(data->files[i])) {
                data->files[i]->error = 1;
                err = 1;
            } else {
                data->files[i]->error = 0;
            }
        }
    }
    END_PROFILE("building symtabs");

    /* TODO - set flags so we can efficiently avoid compiling the bad ones */

    /* Now validate everything. */
    BEGIN_PROFILE();
    for (i = 0; i < data->num_files; ++i) {
        if (!data->files[i]->error
            && yf_validate_ast(
                data, data->files[i], args
            )
        ) {
            data->files[i]->error = 1;
            err = 1;
        }
    }
    END_PROFILE("validating code");

    /* Finally, generate code. */
    if (!args->just_semantics) {
        BEGIN_PROFILE();
        yf_run_backend(data, args);
        gettimeofday(&step_end, NULL);
        END_PROFILE("generating code");
    }
    total_end = step_end;
    if (args->profile) {
        time_total = (total_end.tv_sec - total_begin.tv_sec) * 1000000 +
            (total_end.tv_usec - total_begin.tv_usec);
        YF_PRINT_DEFAULT(
            "Total time: %f seconds",
            ( (double) time_total ) / 1000000.0
        );
    }

#undef BEGIN_PROFILE
#undef END_PROFILE

    return err;

}

static int yf_compile_project(struct yf_args * args) {

    struct yf_project_compilation_data data;
    int ret, i;

    /**
     * Project name is current directory
     */
    data.project_name = yf_malloc(50);
    getcwd(data.project_name, 50);

    if (yf_find_project_files(&data)) {
        return 1;
    }

    if (args->dump_projfiles) {
        YF_PRINT_DEFAULT("Project files: (green = needs to be recompiled):");
        for (i = 0; i < data.num_files; ++i) {
            if (data.files[i]->parse_anew) {
                YF_PRINT_WITH_COLOR(
                    YF_CODE_GREEN,
                    "%s\n",
                    data.files[i]->file_name
                );
            } else {
                YF_PRINT_WITH_COLOR(
                    YF_CODE_YELLOW,
                    "%s\n",
                    data.files[i]->file_name
                );
            }
        }
        return 0;
    }
    
    ret = yf_run_compiler_on_data(&data, args);

    yf_cleanup(&data);

    return ret;

}

static int yf_compile_files(struct yf_args * args) {
    
    struct yf_project_compilation_data data;
    int i, ret;

    /* No project name */
    data.project_name = yf_malloc(50);
    strcpy(data.project_name, "<none>");

    for (i = 0; i < args->num_files; ++i) {
        data.files[i] = malloc(sizeof (struct yf_file_compilation_data));
        memset(data.files[i], 0, sizeof (struct yf_file_compilation_data));
        data.files[i]->file_name = args->files[i];
        data.files[i]->parse_anew = 1;
        /* TODO - more data */
    }

    data.num_files = args->num_files;

    ret = yf_run_compiler_on_data(&data, args);
    yf_cleanup(&data);
    return ret;

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
    char * file_name;

    int retval;

    file_name = file->parse_anew ? file->file_name : file->sym_file;
    file_src = fopen(
        file_name,
        "r"
    );
    if (!file_src) {
        YF_PRINT_ERROR("Could not open file %s", file_name);
        return 1;
    }
    
    input = (struct yf_lexer_input) {
        .input = file_src,
        .getc = (int (*)(void*)) getc,
        .ungetc = (int (*)(int, void*)) ungetc,
        .input_name = file_name,
        .close = (int (*) (void*))fclose
    };

    yfl_init(&lexer, &input);

    if (args->tdump) {
        return dump_tokens(&lexer);
    } else {
        if ( (retval = yf_parse(&lexer, &file->parse_tree)) ) {
            YF_PRINT_ERROR("Error parsing file %s", file->file_name);
            return retval;
        }
        if (args->cstdump) {
            retval = yf_do_cst_dump(&file->parse_tree);
        }
        return retval;
    }

}

/**
 * Stuff the project compilation data with all files that need to be compiled.
 * Returns the number of files.
 */
static int yf_find_project_files(struct yf_project_compilation_data * data) {
    return yfd_find_projfiles(data);
}

/**
 * Dump all file tokens.
 */
static int dump_tokens(struct yf_lexer * lexer) {

    struct yf_token token;
    for (;;) {
        if (yfl_lex(lexer, &token)) {
            YF_PRINT_ERROR("Invalid token");
            return 1;
        }
        if (token.type == YFT_EOF) {
            break;
        }
        printf(
            "%20s, line: %3d, col: %3d, type: %20s\n",
            token.data,
            token.loc.line, token.loc.column, 
            yf_get_toktype(token.type)
        );
    }

    return 0; /* Indicates that nothing should else should be done (meaning no
    semantic analysis, etc. */

}

/**
 * Dump the CST.
 */
static int yf_do_cst_dump(struct yf_parse_node * tree) {
    yf_dump_cst(tree, stderr);
    return 0;
}

/**
 * Build a table of all externally visible symbols.
 */
static int yf_build_symtab(struct yf_file_compilation_data * data) {
    return yfs_build_symtab(data);
}

/**
 * Create an AST from the concrete syntax tree, and validate it against the
 * combined symbol tables loaded so far.
 */
static int yf_validate_ast(
    struct yf_project_compilation_data * pdata,
    struct yf_file_compilation_data * fdata,
    struct yf_args * args
) {

    return yfs_validate(fdata, pdata);

}

/**
 * Destroy all objects and whatnot.
 */
static int yf_cleanup(struct yf_project_compilation_data * data) {

    int iter; /* For all iterations needed */
    struct yf_file_compilation_data * file;
    
    for (iter = 0; iter < data->num_files; ++iter) {

        file = data->files[iter];

        if (file->types.table)
            yf_free(file->types.table);
        if (file->symtab.table)
            yf_free(file->symtab.table);
        yf_cleanup_cst(&file->parse_tree);
        yf_cleanup_ast(&file->ast_tree);
        yf_free(file->output_file);

        yf_free(file);

    }

    yf_free(data->project_name);

    return 0;

}
